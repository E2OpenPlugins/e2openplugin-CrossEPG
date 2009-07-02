#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "db/db.h"
#include "directory.h"

#include "../common.h"
#include "../common/core/log.h"

#include "dgs.h"
#include "dgs_channels.h"
#include "dgs_helper.h"
#include "dgs_scheduler.h"

static dgs_scheduler_t *first_scheduler;
static dgs_scheduler_t *last_scheduler;
static int scheduler_count;

void dgs_scheduler_init ()
{
	first_scheduler = NULL;
	last_scheduler = NULL;
	scheduler_count = 0;
}

void dgs_scheduler_clean ()
{
	dgs_scheduler_t *tmp = first_scheduler;
	
	while (tmp != NULL)
	{
		dgs_scheduler_t *tmp2 = tmp;
		_free (tmp2->name);
		_free (tmp2);
		tmp = tmp->next;
	}
	first_scheduler = NULL;
	last_scheduler = NULL;
	scheduler_count = 0;
}

int _dgs_scheduler_read_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	int *count = (int*)p_data;
	
	dgs_scheduler_t *new_scheduler = _malloc (sizeof (dgs_scheduler_t));
	
	if (first_scheduler == NULL)
	{
		new_scheduler->prev = NULL;
		new_scheduler->next = NULL;
		first_scheduler = new_scheduler;
		last_scheduler = new_scheduler;
	}
	else
	{
		new_scheduler->prev = last_scheduler;
		new_scheduler->next = NULL;
		last_scheduler->next = new_scheduler;
		last_scheduler = new_scheduler;
	}
	
	last_scheduler->id = atoi (p_fields[5]);
	last_scheduler->ch_id = atoi (p_fields[6]);
	last_scheduler->name = _malloc (strlen (p_fields[1]) + 1);
	strcpy (last_scheduler->name, p_fields[1]);
	last_scheduler->mode = atoi (p_fields[0]);
	last_scheduler->start_time = atoi (p_fields[2]);
	last_scheduler->length = atoi (p_fields[3]);
	last_scheduler->is_recording = (atoi (p_fields[4]) == 1);

	*count = *count + 1;
    return 0;
}

bool dgs_scheduler_read ()
{
    int ret;
	int count = 0;
    char *errmsg = 0;

	if (dgs_getdb () == NULL) return false;

	ret = sqlite3_exec (dgs_getdb (), "SELECT mode_type, program_name, start_time, end_time, is_recording, sch_id, ch_id FROM scheduledinfo ORDER BY start_time", _dgs_scheduler_read_callback, &count, &errmsg);
	
	if (ret == SQLITE_OK)
	{
		scheduler_count = count;
		return true;
	}
	else
	{
		log_add ("SQL error: %s", errmsg);
		return false;
	}
}

dgs_scheduler_t *dgs_scheduler_get_first ()
{
	return first_scheduler;
}

dgs_scheduler_t *dgs_scheduler_get_last ()
{
	return last_scheduler;
}

dgs_scheduler_t *dgs_scheduler_get_next (dgs_scheduler_t *scheduler)
{
	if (scheduler != NULL)
	{
		if (scheduler->next != NULL) return scheduler->next;
		else return first_scheduler;
	}
	return NULL;
}

dgs_scheduler_t *dgs_scheduler_get_prev (dgs_scheduler_t *scheduler)
{
	if (scheduler != NULL)
	{
		if (scheduler->prev != NULL) return scheduler->prev;
		else return last_scheduler;
	}
	return NULL;
}

dgs_scheduler_t *dgs_scheduler_get_by_channel_and_title (dgs_channel_t *channel, epgdb_title_t *title)
{
	if (channel == NULL) return NULL;
	if (title == NULL) return NULL;
	
	return dgs_scheduler_get_by_channelid_and_title (channel->id, title);
	/*
	dgs_scheduler_t *scheduler = first_scheduler;
	
	while (scheduler != NULL)
	{
		if (channel->id == scheduler->ch_id)
		{
			if ((dgs_helper_adjust_daylight (title->start_time) >= scheduler->start_time) && ((dgs_helper_adjust_daylight (title->start_time) + title->length) <= (scheduler->start_time + scheduler->length)))
			{
				return scheduler;
			}
		}
		scheduler = scheduler->next;
	}
	
	return NULL;
	*/
}

dgs_scheduler_t *dgs_scheduler_get_by_channelid_and_title (int channel_id, epgdb_title_t *title)
{
	if (title == NULL) return NULL;
	dgs_scheduler_t *scheduler = first_scheduler;
	
	while (scheduler != NULL)
	{
		if (channel_id == scheduler->ch_id)
		{
			if ((dgs_helper_adjust_daylight (title->start_time) >= scheduler->start_time) && ((dgs_helper_adjust_daylight (title->start_time) + title->length) <= (scheduler->start_time + scheduler->length)))
			{
				return scheduler;
			}
		}
		scheduler = scheduler->next;
	}
	
	return NULL;
}

int dgs_scheduler_count ()
{
	return scheduler_count;
}
