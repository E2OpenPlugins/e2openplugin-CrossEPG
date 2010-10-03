#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

#include "../../common.h"

#include "../core/crc32.h"

#include "epgdb.h"
#include "epgdb_index.h"
#include "epgdb_titles.h"

/*
static int _get_mjd (time_t value)
{
	struct tm valuetm;
	int l = 0;
	gmtime_r (&value, &valuetm);
	if (valuetm.tm_mon <= 1)	// Jan or Feb
		l = 1;
	return (14956 + valuetm.tm_mday + ((valuetm.tm_year - l) * 365.25) + ((valuetm.tm_mon + 2 + l * 12) * 30.6001));
}
*/

char *epgdb_read_description (epgdb_title_t *title)
{
	char *ret = _malloc (title->description_length + 1);
	memset (ret, '\0', title->description_length + 1);
	//epgdb_index_t *index = epgdb_index_get_by_crc_length (title->description_crc, title->description_length);
	//if (index == NULL) return ret;
	if (epgdb_get_fdd () == NULL) return ret;
	fseek (epgdb_get_fdd (), title->description_seek, SEEK_SET);
	fread (ret, title->description_length, 1, epgdb_get_fdd ());
	return ret;
}

char *epgdb_read_long_description (epgdb_title_t *title)
{
	char *ret = _malloc (title->long_description_length + 1);
	memset (ret, '\0', title->long_description_length + 1);
	//epgdb_index_t *index = epgdb_index_get_by_crc_length (title->long_description_crc, title->long_description_length);
	//if (index == NULL) return ret;
	if (epgdb_get_fdd () == NULL) return ret;
	
	fseek (epgdb_get_fdd (), title->long_description_seek, SEEK_SET);
	fread (ret, title->long_description_length, 1, epgdb_get_fdd ());
	return ret;
}

epgdb_title_t *epgdb_titles_set_description (epgdb_title_t *title, char *description)
{
	bool added;
	int length = strlen (description);
	uint32_t crc = crc32 ((unsigned char*)description, length);
	if (title->description_length == length && title->description_crc == crc) return title;
	if (!title->changed)
	{
		title->changed = true;
		title->revision++;
	}
	title->description_length = length;
	title->description_crc = crc;
	epgdb_index_t *index = epgdb_index_add (title->description_crc, title->description_length, &added);

	if (added)
	{
		FILE *fd = epgdb_get_fdd ();
		if ((description != NULL) && (fd != NULL))
		{
			fseek (fd, 0, SEEK_END);
			index->length = title->description_length;
			index->seek = ftell (fd);
			fwrite (description, index->length, 1, fd);
		}
	}
	
	title->description_seek = index->seek;
	return title;
}

epgdb_title_t *epgdb_titles_set_long_description (epgdb_title_t *title, char *description)
{
	bool added;
	int length = strlen (description);
	uint32_t crc = crc32 ((unsigned char*)description, length);
	
	if (title->description_length == length && title->description_crc == crc) return title;
	if (!title->changed)
	{
		title->changed = true;
		title->revision++;
	}
	title->long_description_length = length;
	title->long_description_crc = crc;
	epgdb_index_t *index = epgdb_index_add (title->long_description_crc, title->long_description_length, &added);
	
	if (added)
	{
		FILE *fd = epgdb_get_fdd ();
		if ((description != NULL) && (fd != NULL))
		{
			fseek (fd, 0, SEEK_END);
			index->length = title->long_description_length;
			index->seek = ftell (fd);
			fwrite (description, index->length, 1, fd);
		}
	}
	title->long_description_seek = index->seek;
	
	return title;
}

int epgdb_titles_count (epgdb_channel_t *channel)
{
	int count = 0;
	epgdb_title_t *tmp = channel->title_first;
	
	while (tmp != NULL)
	{
		count++;
		tmp = tmp->next;
	}
	
	return count;
}

epgdb_title_t *epgdb_titles_get_by_time (epgdb_channel_t *channel, time_t ttime)
{
	if (channel == NULL) return NULL;
	
	epgdb_title_t *tmp = channel->title_first;
	
	while (tmp != NULL)
	{
		if (tmp->start_time + tmp->length >= ttime) break;
		if (tmp->next == NULL) break;
		tmp = tmp->next;
	}
	
	return tmp;
}

epgdb_title_t *epgdb_titles_get_by_id_and_mjd (epgdb_channel_t *channel, unsigned short int event_id, unsigned short int mjd_time)
{
	if (channel == NULL) return NULL;

	epgdb_title_t *tmp = channel->title_first;
	
	while (tmp != NULL)
	{
		if ((tmp->mjd == mjd_time) && (tmp->event_id == event_id)) break;
		tmp = tmp->next;
	}
	
	return tmp;
}

void epgdb_titles_delete_in_range (epgdb_channel_t *channel, time_t start_time, unsigned short int length)
{
	if (channel == NULL) return;
	
	epgdb_title_t *tmp = channel->title_first;
	
	while (tmp != NULL)
	{
		// do this check better
		if (!(((tmp->start_time + tmp->length) <= start_time) || (tmp->start_time >= (start_time + length))))
		{
			if (tmp->start_time != start_time)
			{
				epgdb_title_t *tmp2 = tmp;
				if (tmp->prev != NULL) tmp->prev->next = tmp->next;
				if (tmp->next != NULL) tmp->next->prev = tmp->prev;
				if (tmp == channel->title_first) channel->title_first = tmp->next;
				if (tmp == channel->title_last) channel->title_last = tmp->prev;
				tmp = tmp->next;
				_free (tmp2);
			}
			else tmp = tmp->next;
		}
		else tmp = tmp->next;
	}
}

epgdb_title_t *epgdb_titles_add (epgdb_channel_t *channel, epgdb_title_t *title)
{
	if (channel == NULL) return NULL;
	if (title == NULL) return NULL;
	
	epgdb_titles_delete_in_range (channel, title->start_time, title->length);
	
	title->description_length = 0;
	title->description_crc = 0;
	title->description_seek = 0;
	title->long_description_length = 0;
	title->long_description_crc = 0;
	title->long_description_seek = 0;
	title->changed = true;
	title->revision = 0;
	
	/* add into list */				
	if (channel->title_first == NULL)
	{
		title->next = NULL;
		title->prev = NULL;
		channel->title_first = title;
		channel->title_last = title;
	}
	else
	{
		epgdb_title_t *tmp = channel->title_first;
		while (true)
		{
			if (tmp->start_time == title->start_time)
			{
				if (tmp->length != title->length ||
					tmp->event_id != title->event_id ||
					tmp->genre_id != title->genre_id)
				{
					tmp->event_id = title->event_id;
					tmp->length = title->length;
					tmp->genre_id = title->genre_id;
					tmp->iso_639_1 = title->iso_639_1;
					tmp->iso_639_2 = title->iso_639_2;
					tmp->iso_639_3 = title->iso_639_3;
					tmp->changed = 1;
					tmp->revision++;
				}
				_free (title);
				title = tmp;
				break;
			}
			if (tmp->start_time > title->start_time)
			{
				title->prev = tmp->prev;
				title->next = tmp;
				title->next->prev = title;
				if (title->prev != NULL)
					title->prev->next = title;
				else
					channel->title_first = title;
				break;
			}
			
			if (tmp->next == NULL)
			{
				title->prev = tmp;
				title->next = NULL;
				title->prev->next = title;
				channel->title_last = title;
				break;
			}
			
			tmp = tmp->next;
		}
	}
	
	return title;
}
