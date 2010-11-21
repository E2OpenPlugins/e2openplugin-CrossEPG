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
	int i;
	for (i = 0; i<sizeof(tmp); i++)
	{
		char test = tmp[i];
		if (test & 0x80)
		{
			useext = true;
			if ((test & 0xE0) != 0xC0 &&
					(test & 0xF0) != 0xE0 &&
					(test & 0xF8) != 0xF0 &&
					(test & 0xFC) != 0xF8 &&
					(test & 0xFE) != 0xFC)
			{
				utferr = true;
			}
		}
	}
	if (!utferr && useext)
		return true;

	return false;
}

parsing_line *createParsingLine (char *value) {
	parsing_line *line = _malloc(sizeof(parsing_line));
	line->pos = 0;
	line->line = value;
	line->field = _malloc(LINE_SIZE);
	line->quotes = false;
	line->ended = false;
	return line;
}

char *csvtok (parsing_line *value, char separator)
{	int i, z;		
	z = 0;
	for (i=value->pos; i<strlen (value->line); i++)
	{
		if ((i == value->pos) && (value->line[i] == '"'))
		{
			value->quotes = true;
			continue;
		}
		if (value->quotes)
		{
			if (value->line[i] == '"') value->quotes = false;
			else if (value->line[i] == '\\')
			{
				i++;
				value->field[z] = value->line[i];
				z++;
			}
			else
			{
				value->field[z] = value->line[i];
				z++;
			}
		}
		else
		{
			if (value->line[i] == separator) break;
			value->field[z] = value->line[i];
			z++;
		}
	}
	value->pos = i+1;
	if (z == 0)
	{
		value->ended = true;
		return "";
	}
	
	value->field[z] = '\0';
	return value->field;
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
	log_add ("Processing %d lines", rows);

	fseek (fd, 0, SEEK_SET);

	count = 0;

	while (fgets (line, sizeof(line), fd) && *stop == false) 
	{
		parsing_line *input = createParsingLine(line);
		int nid = atoi (csvtok (input, ','));
		int tsid = atoi (csvtok (input, ','));
		int sid = atoi (csvtok (input, ','));
		char *tmp;
		epgdb_channel_t *channel = epgdb_channels_add (nid, tsid, sid);
		
		epgdb_title_t *title = _malloc (sizeof (epgdb_title_t));
		title->event_id = event_id;
		title->start_time = atoi (csvtok (input, ','));
		title->length = atoi (csvtok (input, ','));
		title->genre_id = 0;
		title->flags = 0;
		//title->genre_sub_id = 0;
		title->mjd = epgdb_calculate_mjd (title->start_time);
		title->iso_639_1 = 'e';		// default language... if different we set it later
		title->iso_639_2 = 'n';
		title->iso_639_3 = 'g';
		title = epgdb_titles_add (channel, title);

		tmp = csvtok (input, ',');
		if (isUTF8 (tmp))
			SET_UTF8(title->flags);
		epgdb_titles_set_description (title, tmp);

		tmp = csvtok (input, ',');
		if (isUTF8 (tmp))
			SET_UTF8(title->flags);
		epgdb_titles_set_long_description (title, tmp);

		char *iso639 = csvtok (input, ',');
		if (strlen (iso639) >= 3)
		{
			title->iso_639_1 = iso639[0];
			title->iso_639_2 = iso639[1];
			title->iso_639_3 = iso639[2];
		}
		event_id++;
		
		count++;
		if (progress_callback != NULL) progress_callback (count, rows);
		log_add("Parsed: %d line of %d", count, rows);
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

	int max = 1500;

	if (progress_callback != NULL) progress_callback (0, max);
	
	while (fgets (line, sizeof(line), fd)) 
	{
		parsing_line *input = createParsingLine(line);
		char *tmp;
		char nlabel[256];
		int nid = atoi (csvtok (input, ','));
		int tsid = atoi (csvtok (input, ','));
		int sid = atoi (csvtok (input, ','));
		epgdb_channel_t *channel = epgdb_channels_add (nid, tsid, sid);

		//log_add (line);
		
		epgdb_title_t *title = _malloc (sizeof (epgdb_title_t));
		title->event_id = event_id;
		title->start_time = atoi (csvtok (input, ','));
		title->length = atoi (csvtok (input, ','));
		title->genre_id = 0;
		title->flags = 0;
		//title->genre_sub_id = 0;
		title->mjd = epgdb_calculate_mjd (title->start_time);
		title->iso_639_1 = 'e';		// default language... if different we set it later
		title->iso_639_2 = 'n';
		title->iso_639_3 = 'g';
		title = epgdb_titles_add (channel, title);

		tmp = csvtok (input, ',');
		if (isUTF8 (tmp))
			SET_UTF8(title->flags);
		epgdb_titles_set_description (title, tmp);

		tmp = csvtok (input, ',');
		if (isUTF8 (tmp))
			SET_UTF8(title->flags);
		epgdb_titles_set_long_description (title, tmp);
		
		char *iso639 = csvtok (input, ',');
		if (strlen (iso639) >= 3)
		{
			title->iso_639_1 = iso639[0];
			title->iso_639_2 = iso639[1];
			title->iso_639_3 = iso639[2];
		}
		event_id++;
		
		count++;

		if (count > max)
			max++;
		
		sprintf (nlabel, "%s - %d rows parsed", label, count);
		if (file_callback != NULL) file_callback (nlabel);
		if (progress_callback != NULL) progress_callback (count, max);
		log_add("%s", nlabel);
	}
	
	pclose (fd);
	return true;
}
