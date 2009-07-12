#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#ifdef DGS
#include <sqlite3.h>
#endif
#include <pthread.h>
#include <signal.h>

#include "../common.h"

#include "core/log.h"
#include "core/interactive.h"
#include "core/config.h"
#ifdef DGS
#include "../dgs/dgs.h"
#endif
#include "aliases/aliases.h"
#include "epgdb/epgdb.h"
#include "net/http.h"

#include "importer/importer.h"

static char db_root[256];
static char homedir[256];
static char import_root[256];

static volatile bool stop = false;
static volatile bool exec = false;
static volatile bool quit = false;
static volatile bool timeout_enable = true;
pthread_mutex_t mutex;
int timeout = 0;

void progress_callback (int value, int max)
{
	static int last = 0;
	static time_t lasttime = 0;
	int now = (value*100)/max;
	if (now != last && lasttime != time (NULL))
	{
		lasttime = time (NULL);
		interactive_send_int (ACTION_PROGRESS, now);
		last = now;
	}
}

void url_callback (char *url)
{
	interactive_send_text (ACTION_FILE, url);
}

void file_callback (char *file)
{
	interactive_send_text (ACTION_FILE, file);
}

void *import (void *args)
{
	interactive_send (ACTION_START);
	interactive_send_text (ACTION_PROGRESS, "ON");
	importer_parse_directory (import_root, db_root, progress_callback, url_callback, file_callback);
	interactive_send_text (ACTION_PROGRESS, "OFF");
	interactive_send (ACTION_END);
	exec = false;
	return NULL;
}

void *interactive (void *args)
{
	char buffer[4096], byte;
	bool run = true;
	pthread_t thread;
	
	interactive_send (ACTION_READY);
	
	while (run)
	{
		int i = 0, size = 0;
		memset (buffer, '\0', 4096);
		while ((size = fread (&byte, 1, 1, stdin)))
		{
			if (byte == '\n') break;
			buffer[i] = byte; 
			i++;
		}
		
		if (memcmp (buffer, CMD_QUIT, strlen (CMD_QUIT)) == 0 || quit || size == 0)
		{
			run = false;
			stop = true;
		}
		else if (memcmp (buffer, CMD_IMPORT, strlen (CMD_IMPORT)) == 0)
		{
			if (!exec)
			{
				stop = false;
				exec = true;
				pthread_create (&thread, NULL, import, NULL);
			}
			else interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_WAIT, strlen (CMD_WAIT)) == 0)
		{
			timeout_enable = false;
		}
		else if (memcmp (buffer, CMD_SAVE, strlen (CMD_SAVE)) == 0)
		{
			if (!exec)
			{
				timeout_enable = false;
				interactive_send (ACTION_START);
				interactive_send_text (ACTION_PROGRESS, "ON");
				if (!epgdb_save (progress_callback)) interactive_send_text (ACTION_ERROR, "cannot save data");
				interactive_send (ACTION_END);
				interactive_send_text (ACTION_PROGRESS, "OFF");
			}
			else interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_STOP, strlen (CMD_STOP)) == 0)
		{
			stop = true;
			timeout_enable = true;
		}
		else
		{
			interactive_send_text (ACTION_ERROR, "unknow command");
			timeout_enable = true;
		}
		pthread_mutex_lock (&mutex);
		timeout = 0;
		pthread_mutex_unlock (&mutex);
	}
	quit = true;
	if (exec) pthread_join (thread, NULL);
	return NULL;
}

void interactive_manager ()
{
	pthread_t thread;
	quit = false;
	exec = false;
	pthread_mutex_init (&mutex, NULL);
	pthread_create (&thread, NULL, interactive, NULL);
	while (true)
	{
		pthread_mutex_lock (&mutex);
		if (exec || !timeout_enable) timeout = 0;
		else timeout++;
		if (timeout > 200)
		{
			pthread_kill (thread, SIGQUIT);
			quit = true;
		}
		pthread_mutex_unlock (&mutex);
		if (quit) break;
		usleep (100000);
	}
	pthread_join (thread, NULL);
}

int main (int argc, char **argv)
{
	int c, i;
	opterr = 0;
	bool iactive = false;

	strcpy (homedir, argv[0]);
	for (i = strlen (homedir)-1; i >= 0; i--)
	{
		bool ended = false;
		if (homedir[i] == '/') ended = true;
		homedir[i] = '\0';
		if (ended) break;
	}
	
	sprintf (db_root, DEFAULT_DB_ROOT);
	sprintf (import_root, DEFAULT_IMPORT_ROOT);
	
	while ((c = getopt (argc, argv, "d:i:l:k:r")) != -1)
	{
		switch (c)
		{
			case 'd':
				sprintf (db_root, optarg);
				break;
			case 'l':
				sprintf (homedir, optarg);
				break;
			case 'i':
				sprintf (import_root, optarg);
				break;
			case 'k':
				nice (atoi(optarg));
				break;
			case 'r':
				log_disable ();
				interactive_enable ();
				iactive = true;
				break;
			case '?':
				printf ("Usage:\n");
				printf ("  ./crossepg_importer [options]\n");
				printf ("Options:\n");
				printf ("  -d db_root       crossepg db root folder\n");
				printf ("                   default: %s\n", db_root);
				printf ("  -l homedir       home directory\n");
				printf ("                   default: %s\n", homedir);
				printf ("  -i import_root   import root folder\n");
				printf ("                   default: %s\n", import_root);
				printf ("  -k nice          see \"man nice\"\n");
				printf ("  -r               interactive mode\n");
				printf ("  -h               show this help\n");
				return 0;
		}
	}
	
	log_open (NULL, "CrossEPG Importer");
	
	while (homedir[strlen (homedir) - 1] == '/') homedir[strlen (homedir) - 1] = '\0';
	while (db_root[strlen (db_root) - 1] == '/') db_root[strlen (db_root) - 1] = '\0';
	while (import_root[strlen (import_root) - 1] == '/') import_root[strlen (import_root) - 1] = '\0';
	
	if (epgdb_open (db_root)) log_add ("EPGDB opened");
	else
	{
		log_add ("Error opening EPGDB");
		epgdb_close ();
		log_close ();
		return 0;
	}
	epgdb_load ();
	
	aliases_make (homedir);
	
	if (iactive) interactive_manager ();
	else
	{
		importer_parse_directory (import_root, db_root, NULL, NULL, NULL);
		
		log_add ("Saving data...");
		if (epgdb_save (NULL)) log_add ("Data saved");
		else log_add ("Error saving data");
	}
	
	epgdb_clean ();
	memory_stats ();
	log_close ();
	return 0;
}
