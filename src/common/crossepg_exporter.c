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
#include <sys/stat.h>

#include "../common.h"

#include "core/log.h"
#include "core/config.h"
#include "epgdb/epgdb.h"
#include "epgdb/epgdb_channels.h"
#include "epgdb/epgdb_titles.h"
#include "epgdb/epgdb_index.h"


static char db_root[256];

void print_help ()
{
	printf ("Usage:\n");
	printf ("  ./crossepg_exporter [options] output_file\n");
	printf ("Options:\n");
	printf ("  -d db_root       crossepg db root folder\n");
	printf ("                   default: %s\n", db_root);
	printf ("  -k nice          see \"man nice\"\n");
	printf ("  -h               show this help\n\n");
}

//#define MAX_ROW_SIZE (16*1024)	// 16k for row i think is enough
static char *escape_string (char *data)
{
	int i, j = 0;
	char *ret = _malloc ((strlen (data)*2)+1);

	for (i = 0; i < strlen (data); i++)
	{
		if (data[i] == '"')
		{
			ret[j] = '\\';
			ret[j+1] = data[i];
			j += 2;
		}
		else if (data[i] == '\n')
		{
			ret[j] = '\\';
			ret[j+1] = 'n';
			j += 2;
		}
		else
		{
			ret[j] = data[i];
			j++;
		}
	}
	ret[j] = '\0';
	return ret;
}

int main (int argc, char **argv)
{
	int c;
	opterr = 0;
	char *outputfile;
	FILE *fd;
	//char *eventsfile;
	//bool useless = false;

	sprintf (db_root, DEFAULT_DB_ROOT);

	while ((c = getopt (argc, argv, "d:k")) != -1)
	{
		switch (c)
		{
			case 'd':
				sprintf (db_root, optarg);
				break;
			case 'k':
				nice (atoi(optarg));
				break;
			case '?':
				print_help ();
				return 0;
		}
	}
	if (optind != argc-1)
	{
		print_help ();
		return 0;
	}
	else
		outputfile = argv[argc-1];

	log_open (NULL, "CrossEPG Exporter");

	while (db_root[strlen (db_root) - 1] == '/') db_root[strlen (db_root) - 1] = '\0';

	if (epgdb_open (db_root)) log_add ("EPGDB opened (root=%s)", db_root);
	else
	{
		log_add ("Error opening EPGDB");
		epgdb_close ();
		log_close ();
		return 0;
	}
	epgdb_load ();

	log_add ("Exporting data into %s", outputfile);
	//log_add ("Exporting data into");
	//return 0;
	fd = fopen (outputfile, "w");
	if (fd != NULL)
	{
		int count = 0;
		char *desc, *ldesc, *desc_escaped, *ldesc_escaped;

		fprintf (fd, "nid, tsid, sid, start time, length, description, long description, iso693, event id, mjd, genre_id, flags, revision\n");

		epgdb_channel_t *channel = epgdb_channels_get_first ();
		while (channel != NULL)
		{
			epgdb_title_t *title = channel->title_first;
			while (title != NULL)
			{
				//char row[MAX_ROW_SIZE];
				desc = epgdb_read_description (title);
				ldesc = epgdb_read_long_description (title);
				desc_escaped = escape_string (desc);
				ldesc_escaped = escape_string (ldesc);
				fprintf (fd, "%u, %u, %u, %u, %u, \"%s\", \"%s\", \"%c%c%c\", %u, %u, %u, %u, %u\n",
						channel->nid,
						channel->tsid,
						channel->sid,
						title->start_time,
						title->length,
						desc_escaped,
						ldesc_escaped,
						title->iso_639_1,
						title->iso_639_2,
						title->iso_639_3,
						title->event_id,
						title->mjd,
						title->genre_id,
						title->flags,
						title->revision);

				count++;

				_free (desc);
				_free (ldesc);
				_free (desc_escaped);
				_free (ldesc_escaped);
				title = title->next;
			}

			channel = channel->next;
		}
		fclose (fd);
		log_add ("Exported %d events", count);
	}
	else
		log_add ("Cannot open %s", outputfile);

	epgdb_clean ();
	memory_stats ();
	log_close ();
	return 0;
}
