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

#include "../common.h"

#include "core/log.h"
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

int main (int argc, char **argv)
{
	int c, i;
	opterr = 0;

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
	
	log_open (NULL, "CrossEPG Importer");
	
	while ((c = getopt (argc, argv, "d:i:l:")) != -1)
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
				printf ("  -h               show this help\n");
				return 0;
		}
	}
	
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
	
	importer_parse_directory (import_root, db_root, NULL, NULL);
	
	log_add ("Saving data...");
	if (epgdb_save (NULL)) log_add ("Data saved");
	else log_add ("Error saving data");

	epgdb_clean ();
	memory_stats ();
	log_close ();
	return 0;
}
