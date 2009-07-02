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
#ifndef E2
#include <sqlite3.h>
#endif

#include "../../common.h"

#include "../core/log.h"
#include "../core/config.h"
#ifndef E2
#include "../../dgs/dgs.h"
#endif
#include "../aliases/aliases.h"
#include "../epgdb/epgdb.h"
#include "../net/http.h"

#include "csv.h"

static void(*_progress_callback)(char*, int, int) = NULL;
static void(*_progress_url_callback)(char*, int, int) = NULL;
static void(*_step_callback)() = NULL;
static char _file[256];
static char _url[256];

bool importer_extension_check (char *filename, char *extension)
{
	if (strlen (filename) <= strlen (extension) + 1) return false;
	if (memcmp (filename + (strlen (filename) - strlen (extension)), extension, strlen (extension)) != 0) return false;
	if (filename[strlen (filename) - strlen (extension) - 1] != '.') return false;
	return true;
}

void importer_csv_callback (int value, int max)
{
	if (_progress_callback != NULL)
		_progress_callback (_file, value, max);
}

void importer_url_callback (int value, int max)
{
	if (_progress_callback != NULL)
		_progress_url_callback (_url, value, max);
}

void importer_parse_csv (char *dir, char *filename, char *label)
{
	char file[256];
	if (dir == NULL)
		strcpy (file, filename);
	else
		sprintf (file, "%s/%s", dir, filename);
	
	log_add ("Importing data from '%s'...", label);
	if (csv_read (file, importer_csv_callback)) log_add ("Data imported");
	else log_add ("Cannot import csv file");
	
	if (_step_callback != NULL) _step_callback ();
}

void importer_parse_url (char *dir, char *filename, char *dbroot)
{
	FILE *fd;
	char urlfile[256];
	char line[1024];
	
	char host[256];
	char port[6];
	char page[256];
	
	sprintf (urlfile, "%s/%s", dir, filename);
	fd = fopen (urlfile, "r");
	if (fd == NULL)
	{
		log_add ("Cannot read file '%s'", urlfile);
		return;
	}
	
	while (fgets (line, sizeof(line), fd)) 
	{
		char *tmp = line;
		int pos = 0;
		
		strcpy (_url, line);
		
		memset (host, 0, sizeof (host));
		memset (port, 0, sizeof (port));
		memset (page, 0, sizeof (page));
		
		if (strlen (tmp) < 7) continue;
		if (memcmp (tmp, "http://", 7) != 0) continue;
		tmp += 7;
		while ((tmp[0+pos] != '\0') && (tmp[0+pos] != ':') && (tmp[0+pos] != '/')) pos++;
		if (pos > (sizeof (host) - 1)) continue;
		memcpy (host, tmp, pos);
		tmp += pos;
		if (tmp[0] == ':')
		{
			tmp++;
			pos = 0;
			while ((tmp[0+pos] != '\0') && (tmp[0+pos] != '/')) pos++;
			if (pos > (sizeof (port) - 1)) continue;
			memcpy (port, tmp, pos);
			tmp += pos;
		}
		else
			sprintf (port, "80");

		tmp++;
		pos = 0;
		while ((tmp[0+pos] != '\0') && (tmp[0+pos] != '\n')) pos++;
		if (pos > (sizeof (page) - 1)) continue;
		memcpy (page, tmp, pos);
		strcpy (_file, page);
		
		if (line[strlen (line)-1] == '\n') line[strlen (line)-1] = '\0';
		
		if (importer_extension_check (page, "csv"))
		{
			bool ok;
			char sfn[256];
			int fd = -1;
			sprintf (sfn, "%s/crossepg.tmp.XXXXXX", dbroot);
			if ((fd = mkstemp (sfn)) == -1)
			{
				log_add ("Cannot get temp file");
				continue;
			}
			ok = http_get (host, page, atoi (port), fd, importer_url_callback);
			close (fd);
			if (_step_callback != NULL) _step_callback ();
			if (ok) importer_parse_csv (NULL, sfn, line);
			else if (_step_callback != NULL) _step_callback ();
			
			unlink (sfn);
		}
	}
	
	fclose (fd);
}

int importer_set_steps (char *dir, void(*step_callback)())
{
	FILE *fd;
	DIR *dp;
	struct dirent *ep;
	int steps = 0;
	
	_step_callback = step_callback;

	dp = opendir (dir);
	if (dp != NULL)
	{
		while ((ep = readdir (dp)) != NULL)
		{
			if (importer_extension_check (ep->d_name, "csv"))
				steps++;
			else if (importer_extension_check (ep->d_name, "url"))
			{
				char line[1024];
				char urlfile[256];
				sprintf (urlfile, "%s/%s", dir, ep->d_name);
				fd = fopen (urlfile, "r");
				if (fd != NULL)
				{
					while (fgets (line, sizeof(line), fd)) 
					{
						if (memcmp (line, "http://", 7) != 0) continue;
						steps += 2;
					}
					fclose (fd);
				}
			}
		}
		closedir (dp);
	}
	
	return steps;
}

void importer_parse_directory (char *dir, char *dbroot, void(*progress_callback)(char*, int, int), void(*progress_url_callback)(char*, int, int))
{
	DIR *dp;
	struct dirent *ep;
	
	_progress_callback = progress_callback;
	_progress_url_callback = progress_url_callback;
	
	dp = opendir (dir);
	if (dp != NULL)
	{
		while ((ep = readdir (dp)) != NULL)
		{
			if (importer_extension_check (ep->d_name, "csv"))
			{
				char file[256];
				char new_file[256];
				strcpy (_file, ep->d_name);
				importer_parse_csv (dir, ep->d_name, ep->d_name);
				sprintf (file, "%s/%s", dir, ep->d_name);
				sprintf (new_file, "%s.imported", file);
				rename (file, new_file);
			}
			else if (importer_extension_check (ep->d_name, "url"))
				importer_parse_url (dir, ep->d_name, dbroot);
		}
		closedir (dp);
	}
	else
		log_add ("Cannot open import directory '%s'", dir);
}
