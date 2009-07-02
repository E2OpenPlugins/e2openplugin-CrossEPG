#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <wctype.h>

#include "../../common.h"

#include "epgdb.h"
#include "epgdb_channels.h"
#include "epgdb_titles.h"
#include "epgdb_search.h"

void epgdb_search_free (epgdb_search_res_t *value)
{
	_free (value->channels);
	_free (value->titles);
	_free (value);
}

epgdb_search_res_t *epgdb_search_by_name (char *name, int max_results)
{
	int length = strlen (name);
	char tmp[length+1];
	int i;
	for (i=0; i<length; i++) tmp[i] = towupper(name[i]);
	tmp[length] = '\0';
	
	epgdb_search_res_t *ret = _malloc (sizeof (epgdb_search_res_t));
	ret->titles = _malloc (sizeof (epgdb_title_t)*max_results);
	ret->channels = _malloc (sizeof (epgdb_channel_t)*max_results);
	ret->count = 0;
	
	epgdb_channel_t *channel = epgdb_channels_get_first ();
	while (channel != NULL)
	{
		epgdb_title_t *title = channel->title_first;
		
		while (title != NULL)
		{
			if (title->description_length == length)
			{
				char *desc = epgdb_read_description (title);
				if (strlen (desc) == length)
				{
					for (i=0; i<length; i++) desc[i] = towupper(desc[i]);
					if (memcmp (tmp, desc, length) == 0)
					{
						ret->titles[ret->count] = title;
						ret->channels[ret->count] = channel;
						ret->count++;
					}
				}
				_free (desc);
			}
			
			if (ret->count >= max_results) break;
			title = title->next;
		}
		
		if (ret->count >= max_results) break;
		channel = channel->next;
	}
	
	return ret;
}

epgdb_title_t *epgdb_search_by_name_freq_time (char *name, int nid, int tsid, int sid, time_t from_time, time_t to_time)
{
	int length = strlen (name);
	char tmp[length+1];
	int i;
	for (i=0; i<length; i++) tmp[i] = towupper(name[i]);
	tmp[length] = '\0';
	
	epgdb_channel_t *channel = epgdb_channels_get_by_freq (nid, tsid, sid);
	if (channel != NULL)
	{
		epgdb_title_t *title = channel->title_first;
		
		while (title != NULL)
		{
			if ((title->start_time >= from_time) && (title->start_time <= to_time))
			{
				if (title->description_length == length)
				{
					char *desc = epgdb_read_description (title);
					if (strlen (desc) == length)
					{
						for (i=0; i<length; i++) desc[i] = towupper(desc[i]);
						if (memcmp (tmp, desc, length) == 0)
						{
							_free (desc);
							return title;
						}
					}
					_free (desc);
				}
			}
			
			title = title->next;
		}
	}
	
	return NULL;
}
