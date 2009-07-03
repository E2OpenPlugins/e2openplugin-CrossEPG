#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../../common.h"

#include "../../common/core/log.h"
#include "../../common/epgdb/epgdb.h"
#include "../../common/epgdb/epgdb_channels.h"
#include "../../common/epgdb/epgdb_titles.h"
#include "../../common/epgdb/epgdb_search.h"
#include "../dgs_channels.h"
#include "../dgs_scheduler.h"
#include "../dgs_helper.h"

#include "scheduler.h"

#define MAGIC_SCHEDULER		"_xEPG_SCHEDULER"
#define FAKE_SCHEDULER		"__NO__SCHEDULER"
#define SCHEDULER_REVISION	0x00

static scheduler_t *scheduler_first;
static scheduler_t *scheduler_last;

scheduler_t *scheduler_get_first ()
{
	return scheduler_first;
}

void scheduler_init ()
{
	scheduler_first = NULL;
	scheduler_last = NULL;
}

void scheduler_clean ()
{
	scheduler_t *tmp = scheduler_first;
	while (tmp != NULL)
	{
		scheduler_t *tmp2 = tmp;
		tmp = tmp->next;
		if (tmp2->name != NULL) _free (tmp2->name);
		_free (tmp2);
	}
	scheduler_first = NULL;
	scheduler_last = NULL;
}

bool scheduler_load (char *dbroot)
{
	char file[256];
	FILE *fd;
	unsigned char revision;
	char tmp[256];
	int count, i;
	
	sprintf (file, "%s/crossepg.scheduler.db", dbroot);
	fd = fopen (file, "r");
	if (fd == NULL) return false;
	
	fread (tmp, strlen (MAGIC_SCHEDULER), 1, fd);
	if (memcmp (tmp, MAGIC_SCHEDULER, strlen (MAGIC_SCHEDULER)) != 0)
	{
		fclose (fd);
		return false;
	}
	fread (&revision, sizeof (unsigned char), 1, fd);
	if (revision != SCHEDULER_REVISION)
	{
		fclose (fd);
		return false;
	}
	
	fread (&count, sizeof (int), 1, fd);
	for (i=0; i<count; i++)
	{
		scheduler_t *sch = _malloc (sizeof (scheduler_t));
		fread (sch, SCHEDULER_T_SIZE, 1, fd);
		if (sch->name_length > 0)
		{
			sch->name = _malloc (sch->name_length+1);
			fread (sch->name, sch->name_length, 1, fd);
			sch->name[sch->name_length] = '\0';
		}
		else sch->name = NULL;
		
		if (scheduler_first == NULL)
		{
			sch->prev = NULL;
			sch->next = NULL;
			scheduler_first = sch;
			scheduler_last = sch;
		}
		else
		{
			sch->prev = scheduler_last;
			sch->prev->next = sch;
			sch->next = NULL;
			scheduler_last = sch;
		}
	}
	
	fclose (fd);
	return true;
}

bool scheduler_save (char *dbroot)
{
	char file[256];
	FILE *fd;
	
	sprintf (file, "%s/crossepg.scheduler.db", dbroot);
	fd = fopen (file, "w");
	if (fd == NULL) return false;
	
	unsigned char revision = SCHEDULER_REVISION;
	fwrite (FAKE_SCHEDULER, strlen (FAKE_SCHEDULER), 1, fd);
	fwrite (&revision, sizeof (unsigned char), 1, fd);
	
	scheduler_t *tmp = scheduler_first;
	int count = 0;
	while (tmp != NULL)
	{
		count++;
		tmp = tmp->next;
	}
	
	fwrite (&count, sizeof (int), 1, fd);
	
	tmp = scheduler_first;
	while (tmp != NULL)
	{
		fwrite (tmp, SCHEDULER_T_SIZE, 1, fd);
		if (tmp->name_length > 0) fwrite (tmp->name, tmp->name_length, 1, fd);
		tmp = tmp->next;
	}
	
	fflush (fd);
	fsync (fileno (fd));
	fseek (fd, 0, SEEK_SET);
	fwrite (MAGIC_SCHEDULER, strlen (MAGIC_SCHEDULER), 1, fd);
	fflush (fd);
	fclose (fd);
	return true;
}

void scheduler_del (int channel_id, time_t start_time, int length, int type, bool with_dgs)
{
	log_add ("Deleting scheduled record on channel %d at unixtime %d", channel_id, (int)start_time);

	if (with_dgs && (type < 2))
	{
		dgs_scheduler_init ();
		if (dgs_scheduler_read ())
		{
			dgs_scheduler_t *tmp = dgs_scheduler_get_first ();
			while (tmp != NULL)
			{
				if (channel_id == tmp->ch_id)
				{
					if ((start_time >= tmp->start_time) && ((start_time + length) <= (tmp->start_time + tmp->length)))
					{
						dgs_helper_del_scheduler (tmp->id);
						break;
					}
				}
				tmp = tmp->next;
			}
			dgs_scheduler_clean ();
		}
	}
	
	scheduler_t *tmp2 = scheduler_first;
	while (tmp2 != NULL)
	{
		if (channel_id == tmp2->channel_id)
		{
			if ((start_time >= tmp2->start_time) && ((start_time + length) <= (tmp2->start_time + tmp2->length)) && (tmp2->type == type))
			{
				if (tmp2->prev == NULL)
				{
					if (tmp2->next == NULL)
					{
						scheduler_first = NULL;
						scheduler_last = NULL;
					}
					else
					{
						scheduler_first = tmp2->next;
						tmp2->next->prev = NULL;
					}
				}
				else
				{
					if (tmp2->next == NULL)
					{
						scheduler_last = tmp2->prev;
						tmp2->prev->next = NULL;
					}
					else
					{
						tmp2->prev->next = tmp2->next;
						tmp2->next->prev = tmp2->prev;
					}
				}
				if (tmp2->name != NULL) _free (tmp2->name);
				_free (tmp2);
				break;
			}
		}
		tmp2 = tmp2->next;
	}
}

bool scheduler_add (int channel_id, time_t start_time, int length, int type, char *name, bool with_dgs)
{
	if (start_time < time (NULL)) return false;
	if (channel_id <= 0) return false;
	if (length <= 0) return false;
	if ((type != 0) && (type != 1) && (type != 2)) return false;
	
	log_add ("Adding scheduled record on channel %d at unixtime %d", channel_id, (int)start_time);
	bool dgs_added = false;
	if ((with_dgs) && type != 2)
	{
		log_add ("Adding event into DGS scheduler");
		dgs_added = dgs_helper_add_scheduler (channel_id, start_time, length, type, name);
	}
		
	if (dgs_added || !with_dgs || type == 2)
	{
		scheduler_t *item;
		item = _malloc (sizeof (scheduler_t));
		
		item->channel_id = channel_id;
		item->start_time = start_time;
		item->length = length;
		item->type = type;
		if (name != NULL)
		{
			item->name_length = strlen (name);
			item->name = _malloc (item->name_length+1);
			memcpy (item->name, name, item->name_length);
			item->name[item->name_length] = '\0';
		}
		else
		{
			item->name_length = 0;
			item->name = NULL;
		}
		
		/* add into list */
		if (scheduler_first == NULL)
		{
			item->next = NULL;
			item->prev = NULL;
			scheduler_first = item;
			scheduler_last = item;
		}
		else
		{
			scheduler_t *tmp = scheduler_first;
			
			while (true)
			{
				if ((item->start_time == tmp->start_time) && (item->length == tmp->length) && (item->channel_id == tmp->channel_id) && (item->type == tmp->type))
					break;
					
				if (item->type == 2)
				{
					if ((item->start_time < tmp->start_time) && (tmp->type == 2))
					{
						item->prev = tmp->prev;
						item->next = tmp;
						item->next->prev = item;
						if (item->prev != NULL) item->prev->next = item;
						else scheduler_first = item;
						break;
					}
				}
				else
				{
					if ((item->start_time < tmp->start_time) || (tmp->type == 2))
					{
						item->prev = tmp->prev;
						item->next = tmp;
						item->next->prev = item;
						if (item->prev != NULL) item->prev->next = item;
						else scheduler_first = item;
						break;
					}
				}
				
				if (tmp->next == NULL)
				{
					item->prev = tmp;
					item->next = NULL;
					item->prev->next = item;
					scheduler_last = item;
					break;
				}
				
				tmp = tmp->next;
			}
		}
		return true;
	}
	return false;
}

scheduler_t *scheduler_get_by_channel_and_title (dgs_channel_t *channel, epgdb_title_t *title)
{
	if (channel == NULL) return NULL;
	if (title == NULL) return NULL;
	scheduler_t *sch = scheduler_first;
	
	while (sch != NULL)
	{
		if (channel->id == sch->channel_id)
		{
			if ((dgs_helper_adjust_daylight (title->start_time) >= sch->start_time) && ((dgs_helper_adjust_daylight (title->start_time) + title->length) <= (sch->start_time + sch->length)))
			{
				return sch;
			}
		}
		sch = sch->next;
	}
	
	return NULL;
}

bool _scheduler_link_add (int channel_id, epgdb_title_t *title)
{
	bool ret = false;
	time_t now = time (NULL);
	if (dgs_helper_adjust_daylight (title->start_time) > now)
	{
		dgs_scheduler_init ();
		if (dgs_scheduler_read ())
		{
			dgs_scheduler_t *sch = dgs_scheduler_get_by_channelid_and_title (channel_id, title);
			
			if (sch == NULL)
			{
				char *title_text = epgdb_read_description (title);
				if (!scheduler_add (channel_id, dgs_helper_adjust_daylight (title->start_time), title->length, 0, title_text, true))
				{
					// TODO: Add error message box
				}
				else ret = true;
				_free (title_text);
			}

			dgs_scheduler_clean ();
		}
	}
	return ret;
}

void scheduler_link ()
{
	scheduler_t *tmp = scheduler_first;
	
	while (tmp != NULL)
	{
		scheduler_t *next = tmp->next;
		
		if ((tmp->type == 2) && (tmp->name != NULL))
		{
			int nid = 0, tsid = 0, sid = 0;
			int i;
			time_t last_time = tmp->start_time;
			dgs_helper_get_nid_tsid_sid_by_id (tmp->channel_id, &nid, &tsid, &sid);
			
			for (i=0; i<15; i++)
			{
				time_t start_time = tmp->start_time + (i * (60*60*24));
				epgdb_title_t *title = epgdb_search_by_name_freq_time (tmp->name, nid, tsid, sid, (start_time - (dgs_helper_get_daylight_saving () * (60 * 60))) - (60*60), (start_time - (dgs_helper_get_daylight_saving () * (60 * 60))) + (60*60));
				if (title != NULL)
				{
					int skip_count = 0;
					if (_scheduler_link_add (tmp->channel_id, title)) last_time = start_time;
					title = title->next;
					while (title != NULL)
					{
						epgdb_title_t *tmp2 = epgdb_search_by_name_freq_time (tmp->name, nid, tsid, sid, title->start_time, title->start_time);
						if (tmp2 == NULL) skip_count++;
						else
						{
							if (_scheduler_link_add (tmp->channel_id, title)) last_time = start_time;
							skip_count = 0;
						}
						if (skip_count > 1) break;
						title = title->next;
					}
				}
			}
			if ((tmp->start_time == last_time) && (dgs_helper_adjust_daylight (tmp->start_time) < (time (NULL) - (60*60*24*15))))
				scheduler_del (tmp->channel_id, tmp->start_time, tmp->length, tmp->type, false);
			else
			{
				if (tmp->start_time != last_time)
					tmp->start_time = last_time + (60*60*24);
			}
		}
		
		tmp = next;
	}
}
