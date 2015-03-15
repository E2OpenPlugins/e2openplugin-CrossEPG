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
#include <pthread.h>
#include <signal.h>

#include "../common.h"

#include "core/log.h"
#include "core/interactive.h"
#include "aliases/aliases.h"
#include "epgdb/epgdb.h"

static char db_root[256];
static char homedir[256];

static volatile bool db_opened = false;
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

void event_callback (int type, char *arg)
{
	switch (type)
	{
	case 0:
		interactive_send_text (ACTION_PROGRESS, "ON");
		break;
		
	case 1:
		interactive_send_text (ACTION_PROGRESS, "OFF");
		break;
	}
}

bool exec_defrag (bool withcallback)
{
	bool ret;
	char header_filename[256];
	char header_filename_tmp[256];
	char descriptor_filename[256];
	char descriptor_filename_tmp[256];
	char index_filename[256];
	
	sprintf (header_filename, "%s/crossepg.headers.db", db_root);
	sprintf (header_filename_tmp, "%s/crossepg.headers_tmp.db", db_root);
	sprintf (descriptor_filename, "%s/crossepg.descriptors.db", db_root);
	sprintf (descriptor_filename_tmp, "%s/crossepg.descriptors_tmp.db", db_root);
	sprintf (index_filename, "%s/crossepg.indexes.db", db_root);
	
	unlink (header_filename_tmp);
	unlink (descriptor_filename_tmp);
	unlink (index_filename);
	
	rename (header_filename, header_filename_tmp);
	rename (descriptor_filename, descriptor_filename_tmp);
	
	if (!db_opened)
	{
		if (epgdb_open (db_root)) log_add ("EPGDB opened");
		else
		{
			log_add ("Error opening EPGDB");
			return false;
		}
		epgdb_load ();
		db_opened = true;
	}
	if (withcallback)
		ret = dbmerge_fromfile (header_filename_tmp, descriptor_filename_tmp, progress_callback, event_callback, stop);
	else
		ret = dbmerge_fromfile (header_filename_tmp, descriptor_filename_tmp, NULL, NULL, stop);
	
	if (ret)
		log_add ("Data defragmented");
	else
		log_add ("Error defragmenting data");

	unlink (header_filename_tmp);
	unlink (descriptor_filename_tmp);
	
	return ret;
}

void *defrag (void *args)
{
	interactive_send (ACTION_START);
	exec_defrag (true);
	exec = false;
	interactive_send (ACTION_END);
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
		else if (memcmp (buffer, CMD_DEFRAGMENT, strlen (CMD_DEFRAGMENT)) == 0)
		{
			if (!exec)
			{
				stop = false;
				exec = true;
				pthread_create (&thread, NULL, defrag, NULL);
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
			if (!db_opened)
			{
				interactive_send_text (ACTION_ERROR, "cannot do it... epgdb not opened");
			}
			else if (!exec)
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
	
	while ((c = getopt (argc, argv, "d:i:l:k:r")) != -1)
	{
		switch (c)
		{
			case 'd':
				strcpy (db_root, optarg);
				break;
			case 'l':
				strcpy (homedir, optarg);
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
				printf ("  ./crossepg_defragmenter [options]\n");
				printf ("Options:\n");
				printf ("  -d db_root       crossepg db root folder\n");
				printf ("                   default: %s\n", db_root);
				printf ("  -l homedir       home directory\n");
				printf ("                   default: %s\n", homedir);
				printf ("  -k nice          see \"man nice\"\n");
				printf ("  -r               interactive mode\n");
				printf ("  -h               show this help\n");
				return 0;
		}
	}
	
	while (homedir[strlen (homedir) - 1] == '/') homedir[strlen (homedir) - 1] = '\0';
	while (db_root[strlen (db_root) - 1] == '/') db_root[strlen (db_root) - 1] = '\0';
	
	log_open (db_root);
	log_banner ("CrossEPG Defragmenter");

	//aliases_make (homedir);
	
	if (iactive) interactive_manager ();
	else
	{
		if (exec_defrag (false))
		{
			log_add ("Saving data...");
			if (epgdb_save (NULL)) log_add ("Data saved");
			else log_add ("Error saving data");
		}
	}
	
	if (db_opened) epgdb_clean ();
	
	memory_stats ();
	log_close ();
	return 0;
}
