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

int main (int argc, char **argv)
{
	int c, i;
	opterr = 0;

	sprintf (db_root, DEFAULT_DB_ROOT);
	
	log_open (NULL, "CrossEPG DB Info");
	
	while ((c = getopt (argc, argv, "d:")) != -1)
	{
		switch (c)
		{
			case 'd':
				sprintf (db_root, optarg);
				break;
			case '?':
				printf ("Usage:\n");
				printf ("  ./crossepg_importer [options]\n");
				printf ("Options:\n");
				printf ("  -d db_root       crossepg db root folder\n");
				printf ("                   default: %s\n", db_root);
				printf ("  -h               show this help\n");
				return 0;
		}
	}
	
	while (db_root[strlen (db_root) - 1] == '/') db_root[strlen (db_root) - 1] = '\0';
	
	if (epgdb_open (db_root)) log_add ("EPGDB opened");
	else
	{
		log_add ("Error opening EPGDB");
		epgdb_close ();
		log_close ();
		return 0;
	}
	epgdb_load ();
	
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
	format_size (size, getfilesize (descriptors));
	log_add ("Descriptors db size: %s", size);
	format_size (size, getfilesize (indexes));
	log_add ("Indexes db size: %s", size);
	format_size (size, getfilesize (aliases));
	log_add ("Aliases db size: %s", size);
	format_size (size, getfilesize (headers) + getfilesize (descriptors) + getfilesize (aliases) + getfilesize (indexes));
	log_add ("Total size: %s", size);
	log_add ("Channels count: %d", channels_count);
	log_add ("Events count: %d", titles_count);
	log_add ("Hashes count: %d", epgdb_index_count ());
	
	int descriptions_size = 0;
	for (i=0; i<65536; i++)
	{
		epgdb_index_t *index = epgdb_index_get_first (i);
		while (index != NULL)
		{
			descriptions_size += index->length;
			index = index->next;
		}
	}
	format_size (size, descriptions_size);
	log_add ("Descriptions size: %s", size);
	format_size (size, getfilesize (descriptors) - descriptions_size);
	log_add ("Descriptors db unused space: %s", size);

	char mtime[20];
	time_t creation_time = epgdb_get_creation_time ();
	time_t update_time = epgdb_get_update_time ();
	struct tm *loctime = localtime (&creation_time);
	strftime (mtime, 20, "%d/%m/%Y %H:%M", loctime);
	log_add ("Creation time: %s", mtime);
	loctime = localtime (&update_time);
	strftime (mtime, 20, "%d/%m/%Y %H:%M", loctime);
	log_add ("Last update time: %s", mtime);
	
	epgdb_clean ();
	memory_stats ();
	log_close ();
	return 0;
}
