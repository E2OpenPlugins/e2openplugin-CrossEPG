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

static inline bool isUTF8 (char *text)
{
	bool useext = false;
	bool utferr = false;
	char *tmp = text;
	while (tmp != NULL)
	{
		if (*tmp & 0x80)
		{
			useext = true;
			if ((*tmp & 0xE0) != 0xC0 &&
					(*tmp & 0xF0) != 0xE0 &&
					(*tmp & 0xF8) != 0xF0 &&
					(*tmp & 0xFC) != 0xF8 &&
					(*tmp & 0xFE) != 0xFC)
			{
				utferr = true;
			}
		}
		tmp++;
	}
	if (!utferr && useext)
		return true;

	return false;
}

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
	{
		log_add ("Cannot open %s", file);
		return false;
	}

	while (fgets (line, sizeof(line), fd)) rows++;

	fseek (fd, 0, SEEK_SET);

	count = 0;
	while (fgets (line, sizeof(line), fd) && *stop == false) 
	{
		int nid = atoi (csvtok (line, ','));
		int tsid = atoi (csvtok (NULL, ','));
		int sid = atoi (csvtok (NULL, ','));
		char *tmp;
		epgdb_channel_t *channel = epgdb_channels_add (nid, tsid, sid);
		
		epgdb_title_t *title = _malloc (sizeof (epgdb_title_t));
		title->event_id = event_id;
		title->start_time = atoi (csvtok (NULL, ','));
		title->length = atoi (csvtok (NULL, ','));
		title->genre_id = 0;
		title->genre_sub_id = 0;
		title->mjd = get_mjd (title->start_time);
		title->iso_639_1 = 'e';		// default language... if different we set it later
		title->iso_639_2 = 'n';
		title->iso_639_3 = 'g';
		title = epgdb_titles_add (channel, title);

		tmp = csvtok (NULL, ',');
		if (isUTF8 (tmp))
		{
			char *tmp2 = malloc (strlen (tmp) + 2);
			tmp2[0] = 0x15;
			strcpy (tmp2+1, tmp);
			epgdb_titles_set_description (title, tmp2);
			free (tmp2);
		}
		else
			epgdb_titles_set_description (title, tmp);

		tmp = csvtok (NULL, ',');
		if (isUTF8 (tmp))
		{
			char *tmp2 = malloc (strlen (tmp) + 2);
			tmp2[0] = 0x15;
			strcpy (tmp2+1, tmp);
			epgdb_titles_set_long_description (title, tmp2);
			free (tmp2);
		}
		else
			epgdb_titles_set_long_description (title, tmp);
		
		char *iso639 = csvtok (NULL, ',');
		if (strlen (iso639) >= 3)
		{
			title->iso_639_1 = iso639[0];
			title->iso_639_2 = iso639[1];
			title->iso_639_3 = iso639[2];
		}
		event_id++;
		
		count++;
		if (progress_callback != NULL) progress_callback (count, rows);
	}
	
	fclose (fd);
	return true;
}

bool bin_read (char *file, char *label, void(*progress_callback)(int, int), void(*file_callback)(char*))
{
	char line[LINE_SIZE];
	FILE *fd;
	int event_id = 0;
	//int rows = 0;
	int count = 0;

	fd = popen (file, "r");
	if (!fd) 
	{
		log_add ("Cannot open %s", file);
		return false;
	}

	if (progress_callback != NULL) progress_callback (0, 0);
	
	while (fgets (line, sizeof(line), fd)) 
	{
		char nlabel[256];
		int nid = atoi (csvtok (line, ','));
		int tsid = atoi (csvtok (NULL, ','));
		int sid = atoi (csvtok (NULL, ','));
		epgdb_channel_t *channel = epgdb_channels_add (nid, tsid, sid);

		//log_add (line);
		
		epgdb_title_t *title = _malloc (sizeof (epgdb_title_t));
		title->event_id = event_id;
		title->start_time = atoi (csvtok (NULL, ','));
		title->length = atoi (csvtok (NULL, ','));
		title->genre_id = 0;
		title->genre_sub_id = 0;
		title->mjd = get_mjd (title->start_time);
		title->iso_639_1 = 'e';		// default language... if different we set it later
		title->iso_639_2 = 'n';
		title->iso_639_3 = 'g';
		title = epgdb_titles_add (channel, title);

		epgdb_titles_set_description (title, csvtok (NULL, ','));
		epgdb_titles_set_long_description (title, csvtok (NULL, ','));
		
		char *iso639 = csvtok (NULL, ',');
		if (strlen (iso639) >= 3)
		{
			title->iso_639_1 = iso639[0];
			title->iso_639_2 = iso639[1];
			title->iso_639_3 = iso639[2];
		}
		event_id++;
		
		count++;
		
		sprintf (nlabel, "%s - %d rows parsed", label, count);
		if (file_callback != NULL) file_callback (nlabel);
		if (progress_callback != NULL) progress_callback (0, 0);
	}
	
	pclose (fd);
	return true;
}
