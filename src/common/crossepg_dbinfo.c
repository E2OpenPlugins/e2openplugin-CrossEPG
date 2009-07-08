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

#include "../common.h"

#include "core/log.h"
#include "core/config.h"
#include "core/interactive.h"
#include "epgdb/epgdb.h"
#include "epgdb/epgdb_channels.h"
#include "epgdb/epgdb_titles.h"
#include "epgdb/epgdb_index.h"

static char db_root[256];

static void format_size (char *string, int size)
{
	if (size > (1024*1024))
	{
		int sz = size / (1024*1024);
		int dc = (size % (1024*1024)) / (1024*10);
		if (dc > 0)
		{
			if (dc < 10)
				sprintf (string, "%d.0%d MB", sz, dc);
			else if (dc < 100)
				sprintf (string, "%d.%d MB", sz, dc);
			else
				sprintf (string, "%d.99 MB", sz);
		}
		else
			sprintf (string, "%d MB", sz);
	}
	else if (size > 1024)
		sprintf (string, "%d KB", (size / 1024));
	else
		sprintf (string, "%d bytes", size);
}

int getfilesize (char *filename)
{
	FILE *fd = fopen (filename, "r");
	if (fd != NULL)
	{
		fseek (fd, 0, SEEK_END);
		int size = ftell (fd);
		fclose (fd);
		return size;
	}
	else return 0;
}

void dump ()
{
	epgdb_channel_t *channel = epgdb_channels_get_first ();

	while (channel != NULL)
	{
		printf ("Service ID: %X Transport Service ID: %X Network ID: %X\n", channel->sid, channel->tsid, channel->nid);
		epgdb_title_t *title = channel->title_first;
		while (title != NULL)
		{
			printf ("Start time: %li Length %d\n", title->start_time, title->length);
			char *description = epgdb_read_description (title);
			char *ldescription = epgdb_read_long_description (title);
			printf ("%s\n", description);
			printf ("%s\n", ldescription);
			_free (description);
			_free (ldescription);
			title = title->next;
		}
		channel = channel->next;
	}
}

int main (int argc, char **argv)
{
	int c, i;
	opterr = 0;
	bool iactive = false;
	bool edump = false;

	sprintf (db_root, DEFAULT_DB_ROOT);
	
	while ((c = getopt (argc, argv, "d:k:rt")) != -1)
	{
		switch (c)
		{
			case 'd':
				sprintf (db_root, optarg);
				break;
			case 'k':
				nice (atoi(optarg));
				break;
			case 'r':
				log_disable ();
				interactive_enable ();
				iactive = true;
				break;
			case 't':
				edump = true;
				break;
			case '?':
				printf ("Usage:\n");
				printf ("  ./crossepg_importer [options]\n");
				printf ("Options:\n");
				printf ("  -d db_root       crossepg db root folder\n");
				printf ("                   default: %s\n", db_root);
				printf ("  -t               dump the database in text format (not interactive)\n");
				printf ("  -k nice          see \"man nice\"\n");
				printf ("  -r               interactive mode\n");
				printf ("  -h               show this help\n");
				return 0;
		}
	}
	
	if (iactive) edump = false;
	
	log_open (NULL, "CrossEPG DB Info");
	
	while (db_root[strlen (db_root) - 1] == '/') db_root[strlen (db_root) - 1] = '\0';
	
	mkdir (db_root, S_IRWXU|S_IRWXG|S_IRWXO);
	
	if (epgdb_open (db_root)) log_add ("EPGDB opened");
	else
	{
		log_add ("Error opening EPGDB");
		epgdb_close ();
		log_close ();
		return 0;
	}
	epgdb_load ();
	
	if (edump) dump ();
	else
	{
		interactive_send_text (INFO_VERSION, RELEASE);

		char mtime[20];
		time_t creation_time = epgdb_get_creation_time ();
		time_t update_time = epgdb_get_update_time ();
		struct tm *loctime = localtime (&creation_time);
		strftime (mtime, 20, "%d/%m/%Y %H:%M", loctime);
		log_add ("Creation time: %s", mtime);
		interactive_send_text (INFO_CREATION_TIME, mtime);
		loctime = localtime (&update_time);
		strftime (mtime, 20, "%d/%m/%Y %H:%M", loctime);
		log_add ("Last update time: %s", mtime);
		interactive_send_text (INFO_UPDATE_TIME, mtime);

		epgdb_channel_t *channel = epgdb_channels_get_first ();

		int channels_count = 0;
		int titles_count = 0;
		while (channel != NULL)
		{
			epgdb_title_t *title = channel->title_first;
			while (title != NULL)
			{
				titles_count++;
				title = title->next;
			}
			channels_count++;
			channel = channel->next;
		}
		char headers[1024];
		char descriptors[1024];
		char aliases[1024];
		char indexes[1024];
		char size[256];

		sprintf (headers, "%s/crossepg.headers.db", db_root);
		sprintf (descriptors, "%s/crossepg.descriptors.db", db_root);
		sprintf (aliases, "%s/crossepg.aliases.db", db_root);
		sprintf (indexes, "%s/crossepg.indexes.db", db_root);

		format_size (size, getfilesize (headers));
		log_add ("Headers db size: %s", size);
		interactive_send_text (INFO_HEADERSDB_SIZE, size);
		format_size (size, getfilesize (descriptors));
		log_add ("Descriptors db size: %s", size);
		interactive_send_text (INFO_DESCRIPTORSDB_SIZE, size);
		format_size (size, getfilesize (indexes));
		log_add ("Indexes db size: %s", size);
		interactive_send_text (INFO_INDEXESDB_SIZE, size);
		format_size (size, getfilesize (aliases));
		log_add ("Aliases db size: %s", size);
		interactive_send_text (INFO_ALIASESDB_SIZE, size);
		format_size (size, getfilesize (headers) + getfilesize (descriptors) + getfilesize (aliases) + getfilesize (indexes));
		log_add ("Total size: %s", size);
		interactive_send_text (INFO_TOTAL_SIZE, size);
		log_add ("Channels count: %d", channels_count);
		interactive_send_int (INFO_CHANNELS_COUNT, channels_count);
		log_add ("Events count: %d", titles_count);
		interactive_send_int (INFO_EVENTS_COUNT, titles_count);
		log_add ("Hashes count: %d", epgdb_index_count ());
		interactive_send_int (INFO_HASHES_COUNT, epgdb_index_count ());
	}
	
	epgdb_clean ();
	memory_stats ();
	log_close ();
	return 0;
}
