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
#include "core/interactive.h"
#include "epgdb/epgdb.h"
#include "epgdb/epgdb_channels.h"
#include "epgdb/epgdb_titles.h"
#include "epgdb/epgdb_index.h"
#include "xmltv/xmltv_channels.h"
#include "xmltv/xmltv_parser.h"

static char db_root[256];

void print_help ()
{
	printf ("Usage:\n");
	printf ("  ./crossepg_xmltv [options] channels_file events_file\n");
	printf ("Options:\n");
	printf ("  -d db_root       crossepg db root folder\n");
	printf ("                   default: %s\n", db_root);
	printf ("  -k nice          see \"man nice\"\n");
	printf ("  -h               show this help\n");
}


int main (int argc, char **argv)
{
	int c;
	opterr = 0;
	char *channelsfile;
	char *eventsfile;

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
	if (optind != argc-2)
	{
		print_help ();
		return 0;
	}
	else
	{
		channelsfile = argv[argc-2];
		eventsfile = argv[argc-1];
	}
	
	log_open (NULL, "CrossEPG XMLTV Importer");
	
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
	
	xmltv_channels_init ();
	xmltv_channels_load (channelsfile);
	xmltv_parser_import (eventsfile);
	xmltv_channels_cleanup ();
	
	log_add ("Saving data");
	if (epgdb_save (NULL)) log_add ("Data saved");
	else log_add ("Error saving data");
	
	epgdb_clean ();
	memory_stats ();
	log_close ();
	return 0;
}
