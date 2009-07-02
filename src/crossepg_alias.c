#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef E2
#include <sqlite3.h>
#endif

#include "common.h"

#include "core/log.h"
#include "core/config.h"
#ifndef E2
#include "dgs/dgs.h"
#include "dgs/dgs_helper.h"
#endif

#include "aliases/aliases.h"

#include "epgdb/epgdb.h"
#include "epgdb/epgdb_channels.h"

static char db_root[256]; // = DEFAULT_DB_ROOT;
static char homedir[256]; // = DEFAULT_HOME_DIRECTORY;
static char alias_name[256];

void show_help ()
{
	printf ("Usage:\n");
	printf ("  ./crossepg_alias action [options]\n");
	printf ("Options:\n");
	printf ("  -d db_root    crossepg db root folder\n");
	printf ("                default: %s\n", db_root);
	printf ("  -l homedir    home directory\n");
	printf ("                default: %s\n", homedir);
	printf ("  -a name       alias name\n");
	printf ("                default: %s\n", alias_name);
	printf ("  -h            show this help\n");
	printf ("Actions:\n");
	printf ("  auto [grp_id]                     automatic add all channels with the same name\n");
	printf ("  add [grp_id:]ch_a [grp_id:]ch_b   add an alias from channel A to channel B\n");
	printf ("  make                              build the binary db crossepg.aliases.db\n");
}

int main (int argc, char **argv)
{
	int c, offset = 0, group = -1;
	bool optmake = false;
	bool optauto = false;
	bool optadd = false;
	char channel_a[256];
	char channel_b[256];
	char file[256];
	
	opterr = 0;
	
	sprintf (db_root, DEFAULT_DB_ROOT);
	sprintf (homedir, DEFAULT_HOME_DIRECTORY);
	sprintf (alias_name, "default");
	
	log_open (NULL, "CrossEPG Alias");
	
	if (argc < 2)
	{
		show_help ();
		return 0;
	}
	
	if (memcmp (argv[1], "auto", 4) == 0)
	{
		offset = 1;
		optauto = true; 
		if (argc > 2)
		{
			if (strlen (argv[2]) > 0)
			{
				if (argv[2][0] != '-')
				{
					group = atoi (argv[2]);
					offset = 2;
				}
			}
		}
	}
	else if (memcmp (argv[1], "make", 4) == 0)
	{
		offset = 1;
		optmake = true;
	}
	else if (memcmp (argv[1], "add", 4) == 0)
	{
		offset = 3;
		optadd = true;
		if (argc < 4)
		{
			show_help ();
			return 0;
		}
		strcpy (channel_a,  argv[2]);
		strcpy (channel_b,  argv[3]);
	}
	else
	{
		show_help ();
		return 0;
	}
	
	while ((c = getopt (argc-offset, argv+offset, "d:l:a:")) != -1)
	{
		switch (c)
		{
			case 'd':
				sprintf (db_root, optarg);
				break;
			case 'l':
				sprintf (homedir, optarg);
				break;
			case 'a':
				sprintf (alias_name, optarg);
				break;
			case '?':
				show_help ();
				return 0;
		}
	}
	
	while (homedir[strlen (homedir) - 1] == '/') homedir[strlen (homedir) - 1] = '\0';
	while (db_root[strlen (db_root) - 1] == '/') db_root[strlen (db_root) - 1] = '\0';
	
#ifndef E2
	if (!dgs_opendb ())
	{
		log_add ("Cannot open DGS db");
		return 0;
	}
#endif

	if (epgdb_open (db_root)) log_add ("EPGDB opened");
	else
	{
		log_add ("Error opening EPGDB");
		epgdb_close ();
#ifndef E2
		dgs_closedb();
#endif
		log_close ();
		return 0;
	}
	epgdb_load ();
	
	sprintf (file, "%s/aliases/%s.conf", homedir, alias_name);
	
	if (optauto)
		aliases_auto (group, file);

	if (optadd)
		aliases_add (channel_a, channel_b, file);

	if (optmake)
	{
		aliases_make (homedir);
		log_add ("Saving data...");
		if (epgdb_save (NULL)) log_add ("Data saved");
		else log_add ("Error saving data");
	}
	
	epgdb_clean ();
#ifndef E2
	dgs_closedb();
#endif
	memory_stats ();
	log_close ();
	return 0;
}
