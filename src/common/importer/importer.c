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
#include "gzip.h"
#include "csv.h"

static void(*_progress_callback)(int, int) = NULL;
static void(*_url_callback)(char*) = NULL;
static void(*_file_callback)(char*) = NULL;
static void(*_step_callback)() = NULL;
static char _file[256];
static char _url[256];
static volatile bool *_stop;

bool importer_extension_check (char *filename, char *extension)
{
	if (strlen (filename) <= strlen (extension) + 1) return false;
	if (memcmp (filename + (strlen (filename) - strlen (extension)), extension, strlen (extension)) != 0) return false;
	if (filename[strlen (filename) - strlen (extension) - 1] != '.') return false;
	return true;
}

void importer_parse_csv (char *dir, char *filename, char *label)
{
	char file[256];
	if (dir == NULL)
		strcpy (file, filename);
	else
		sprintf (file, "%s/%s", dir, filename);
	
	log_add ("Importing data from '%s'...", label);
	if (_file_callback != NULL) _file_callback (label);
	if (csv_read (file, _progress_callback, _stop)) log_add ("Data imported");
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
		
		if (line[strlen (line)-1] == '\n') line[strlen (line)-1] = '\0';
		
		if (importer_extension_check (page, "csv") || importer_extension_check (page, "csv.gz"))
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
			char tmp_url[256];
			sprintf (tmp_url, "http://%s/%s", host, page);
			if (_url_callback != NULL) _url_callback (tmp_url);
			ok = http_get (host, page, atoi (port), fd, _progress_callback, _stop);
			close (fd);
			if (_step_callback != NULL) _step_callback ();
			if (ok)
			{
				if (importer_extension_check (page, "csv")) importer_parse_csv (NULL, sfn, line);
				else if (importer_extension_check (page, "csv.gz"))
				{
					int fd2 = -1;
					char sfn2[256];
					sprintf (sfn2, "%s/crossepg.tmp.XXXXXX", dbroot);
					if ((fd2 = mkstemp (sfn2)) == -1) log_add ("Cannot get temp file");
					else
					{
						FILE *dest = fdopen (fd2, "w");
						if (!gzip_inf (sfn, dest)) log_add ("Error deflating file");
						fclose (dest);
						close (fd2);
						importer_parse_csv (NULL, sfn2, line);
						unlink (sfn2);
					}
				}
			}
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

void importer_parse_directory (char *dir, char *dbroot,
								void(*progress_callback)(int, int),
								void(*url_callback)(char*),
								void(*file_callback)(char*),
								volatile bool *stop)
{
	DIR *dp;
	struct dirent *ep;
	
	_progress_callback = progress_callback;
	_url_callback = url_callback;
	_file_callback = file_callback;
	_stop = stop;
	
	dp = opendir (dir);
	if (dp != NULL)
	{
		while ((ep = readdir (dp)) != NULL && *stop == false)
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
			else if (importer_extension_check (ep->d_name, "csv.gz"))
			{
				char file[256];
				char new_file[256];
				int fd = -1;
				char sfn[256];
				strcpy (_file, ep->d_name);
				sprintf (sfn, "%s/crossepg.tmp.XXXXXX", dbroot);
				if ((fd = mkstemp (sfn)) == -1) log_add ("Cannot get temp file");
				else
				{
					FILE *dest = fdopen (fd, "w");
					if (!gzip_inf (ep->d_name, dest)) log_add ("Error deflating file");
					fclose (dest);
					close (fd);
					importer_parse_csv (NULL, sfn, ep->d_name);
					unlink (sfn);
				}
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
