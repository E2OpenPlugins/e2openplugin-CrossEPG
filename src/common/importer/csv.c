#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>

#include "../../common.h"

#include "../core/log.h"
#include "../epgdb/epgdb.h"
#include "../epgdb/epgdb_channels.h"
#include "../epgdb/epgdb_titles.h"

#include "csv.h"

#define LINE_SIZE 32*1024	//32k

char *csvtok (char *value, char separator)
{
	static char line[LINE_SIZE];
	static char field[LINE_SIZE];
	static int pos;
	static bool quotes;
	static bool ended;
	int i, z;
	
	if (value != NULL)
	{
		strcpy (line, value);
		pos = 0;
		quotes = false;
		ended = false;
	}
	z=0;
	for (i=pos; i<strlen (line); i++)
	{
		if ((i == pos) && (line[i] == '"'))
		{
			quotes = true;
			continue;
		}
		if (quotes)
		{
			if (line[i] == '"') quotes = false;
			else if (line[i] == '\\')
			{
				i++;
				field[z] = line[i];
				z++;
			}
			else
			{
				field[z] = line[i];
				z++;
			}
		}
		else
		{
			if (line[i] == separator) break;
			field[z] = line[i];
			z++;
		}
	}
	pos = i+1;
	if (z == 0)
	{
		ended = true;
		return "";
	}
	
	field[z] = '\0';
	return field;
}

static int get_mjd (time_t value)
{
	struct tm *time = gmtime(&value);
	
	int l = 0;
	int month = time->tm_mon + 1;
	if (month == 1 || month == 2)
		l = 1;
	return 14956 + time->tm_mday + (int)((time->tm_year - l) * 365.25) + (int)((month + 1 + l*12) * 30.6001);
}

bool csv_read (char *file, void(*progress_callback)(int, int), volatile bool *stop)
{
	char line[LINE_SIZE];
	FILE *fd;
	int event_id = 0;
	int rows = 0;
	int count = 0;
	
	fd = fopen (file, "r");
	if (!fd) 
		return false;

	while (fgets (line, sizeof(line), fd)) rows++;

	fseek (fd, 0, SEEK_SET);

	count = 0;
	while (fgets (line, sizeof(line), fd) && *stop == false) 
	{
		int nid = atoi (csvtok (line, ','));
		int tsid = atoi (csvtok (NULL, ','));
		int sid = atoi (csvtok (NULL, ','));
		epgdb_channel_t *channel = epgdb_channels_add (nid, tsid, sid);
		
		epgdb_title_t *title = _malloc (sizeof (epgdb_title_t));
		title->event_id = event_id;
		title->start_time = atoi (csvtok (NULL, ','));
		title->length = atoi (csvtok (NULL, ','));
		title->genre_id = 0;
		title->genre_sub_id = 0;
		title->mjd = get_mjd (title->start_time);
		epgdb_titles_add (channel, title);
		
		epgdb_titles_set_description (title, csvtok (NULL, ','));
		epgdb_titles_set_long_description (title, csvtok (NULL, ','));
		event_id++;
		
		count++;
		if (progress_callback != NULL) progress_callback (count, rows);
	}
	
	fclose (fd);
	return true;
}

