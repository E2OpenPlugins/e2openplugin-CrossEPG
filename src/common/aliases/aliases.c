#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#ifndef E2
#include <sqlite3.h>
#endif
#include <wctype.h>
#include <dirent.h>

#include "../../common.h"

#include "../core/log.h"
#ifndef E2
#include "../../dgs/dgs.h"
#include "../../dgs/dgs_helper.h"
#endif
#include "../epgdb/epgdb.h"
#include "../epgdb/epgdb_channels.h"
#include "../epgdb/epgdb_titles.h"
#include "../epgdb/epgdb_aliases.h"

#include "aliases.h"

typedef struct alias_s
{
	char 			 		*name;
	unsigned short int		nid[32];
	unsigned short int 		tsid[32];
	unsigned short int 		sid[32];
	int						count;
} alias_t;

static int aliases_count = 0;

bool _aliases_auto_save (alias_t *aliases, char *filename)
{
	FILE *fd;
	int i, j;
	
	fd = fopen (filename, "w");
	
	if (fd == NULL) return false;
	
	for (i=0; i<aliases_count; i++)
	{
		if (aliases[i].count > 1)
		{
			char tmp[256];
			sprintf (tmp, "# %s (auto)\n", aliases[i].name);
			fwrite (tmp, strlen (tmp), 1, fd);
			for (j=0; j<aliases[i].count; j++)
			{
				if (j != 0) fwrite (",", 1, 1, fd);
				sprintf (tmp, "%d|%d|%d", aliases[i].nid[j], aliases[i].tsid[j], aliases[i].sid[j]);
				fwrite (tmp, strlen (tmp), 1, fd);
			}
			fwrite ("\n", 1, 1, fd);
			fflush (fd);
		}
	}
	
	fclose (fd);
	
	return true;
}

char *aliastok (char *value)
{
	static char line[1024];
	static char field[1024];
	static int pos;
	int i, z;
	
	if (value != NULL)
	{
		strcpy (line, value);
		pos = 0;
	}
	z=0;
	for (i=pos; i<strlen (line); i++)
	{
		if (line[i] == ',') break;
		field[z] = line[i];
		z++;
	}
	pos = i+1;
	if (z == 0) return NULL;
	
	field[z] = '\0';
	return field;
}

static char *_aliases_trim_spaces (char *text)
{
	char *tmp = text;
	while (tmp[0] == ' ') tmp++;
	while (strlen (tmp) > 1)
		if (tmp[strlen (tmp) - 1] == ' ') tmp[strlen (tmp) - 1] = '\0';
		else break;
	
	if (tmp[0] == ' ') tmp[0] = '\0';
	return tmp;
}

int _aliases_get_id (alias_t *aliases /* alias_t array */, alias_t *alias /* alias_t pointer */)
{
	int i, j, k, old_id = -1;
	for (k=0; k<alias->count; k++)
	{
		for (i=0; i<aliases_count; i++)
		{
			for (j=0; j<aliases[i].count; j++)
			{
				if ((alias->nid[k] == aliases[i].nid[j]) && (alias->tsid[k] == aliases[i].tsid[j]) && (alias->sid[k] == aliases[i].sid[j]))
				{
					if (old_id == -1) old_id = i;
					else if (old_id != i) return -2;
				}
			}
		}
	}
	
	return old_id;
}

void _aliases_load (alias_t *aliases, int max_aliases, char *filename)
{
	char line[1024];
	FILE *fd;
	
	fd = fopen (filename, "r");
	if (fd == NULL)
	{
		log_add ("Cannot load aliases from file '%s'", filename);
		return;
	}
	
	while (fgets (line, sizeof (line), fd)) 
	{
		if (line[strlen (line)-1] == '\n') line[strlen (line)-1] = '\0';
		char *tmp = _aliases_trim_spaces (line);
		if (tmp[0] == '#') continue;
				
		alias_t alias;
		alias.count = 0;
		char *tmp2 = aliastok (tmp);
		if (tmp2 != NULL)
		{
			do
			{
				char* tmp3 = strtok (tmp2, "|");		// nid
				if (tmp3 == NULL) break;
				alias.nid[alias.count] = atoi (tmp3);
				tmp3 = strtok (NULL, "|");				// tsid
				if (tmp3 == NULL) break;
				alias.tsid[alias.count] = atoi (tmp3);
				tmp3 = strtok (NULL, "|");				// sid
				if (tmp3 == NULL) break;
				alias.sid[alias.count] = atoi (tmp3);
				alias.count++;
			}
			while ((tmp2 = aliastok (NULL)) != NULL);
		}
		
		if (alias.count > 1)
		{
			int id = _aliases_get_id (aliases, &alias);
			if (id == -1)
			{
				int i;
				for (i=0; i<alias.count; i++)
				{
					aliases[aliases_count].nid[i] = alias.nid[i];
					aliases[aliases_count].tsid[i] = alias.tsid[i];
					aliases[aliases_count].sid[i] = alias.sid[i];
				}
				aliases[aliases_count].count = alias.count;
				aliases_count++;
			}
			else if (id == -2) log_add ("WARNING... some aliases are skipped due a conflict with other aliases");
			else
			{
				int i;
				for (i=0; i<alias.count; i++)
				{
					aliases[id].nid[aliases[id].count] = alias.nid[i];
					aliases[id].tsid[aliases[id].count] = alias.tsid[i];
					aliases[id].sid[aliases[id].count] = alias.sid[i];
					aliases[id].count++;
					if (aliases[id].count > 31) break;
				}
			}
		}
		else
			log_add ("WARNING... aliases configuration file may be malfored");
		
		if (aliases_count >= max_aliases) break;
	}
	
	fclose (fd);
}

#ifdef E2
#include "aliases.enigma2.c"
#else
#include "aliases.dgs.c"
#endif

bool _aliases_make_extension_check (char *filename, char *extension)
{
	if (strlen (filename) <= strlen (extension) + 1) return false;
	if (memcmp (filename + (strlen (filename) - strlen (extension)), extension, strlen (extension)) != 0) return false;
	if (filename[strlen (filename) - strlen (extension) - 1] != '.') return false;
	return true;
}

void aliases_make (char *home)
{
	char dir[256];
	DIR *dp;
	struct dirent *ep;
	alias_t aliases[1024];
	int i, j;
	
	log_add ("Clearing old aliases...");
	epgdb_aliases_clear ();
	
	sprintf (dir, "%s/aliases", home);
	dp = opendir (dir);
	if (dp != NULL)
	{
		while ((ep = readdir (dp)) != NULL)
		{
			if (_aliases_make_extension_check (ep->d_name, "conf"))
			{
				char filename[256];
				log_add ("Loading '%s'...", ep->d_name);
				sprintf (filename, "%s/%s", dir, ep->d_name);
				_aliases_load (aliases, 1024, filename);
			}
		}
		closedir (dp);
		
		log_add ("Loaded %d aliases", aliases_count);
		log_add ("Adding new aliases...");
		
		for (i=0; i<aliases_count; i++)
		{
			if (aliases[i].count > 1)
			{
				epgdb_channel_t *channel = NULL;
				for (j=0; j<aliases[i].count; j++)
				{
					channel = epgdb_channels_get_by_freq (aliases[i].nid[j], aliases[i].tsid[j], aliases[i].sid[j]);
					if (channel != NULL) break;
				}
				
				if (channel == NULL)
					channel = epgdb_channels_add (aliases[i].nid[0], aliases[i].tsid[0], aliases[i].sid[0]);
				
				if (channel != NULL)
				{
					for (j=0; j<aliases[i].count; j++)
					{
						epgdb_channel_t *tmp = epgdb_aliases_add (channel, aliases[i].nid[j], aliases[i].tsid[j], aliases[i].sid[j]);
						if ((tmp != NULL) && (tmp != channel))
							log_add ("Cannot add alias due an internal conflict (nid: %d tsid: %d sid: %d)", aliases[i].nid[j], aliases[i].tsid[j], aliases[i].sid[j]);
					}
				}
			}
		}
		log_add ("Completed");
	}
	else
		log_add ("Cannot open aliases directory '%s'", dir);
}
