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

static dgs_channel_t *first_channel;
static dgs_channel_t *last_channel;
static int channels_count;

void dgs_channels_init ()
{
	first_channel = NULL;
	last_channel = NULL;
	channels_count = 0;
}

void dgs_channels_clean ()
{
	dgs_channel_t *tmp = first_channel;
	
	while (tmp != NULL)
	{
		dgs_channel_t *tmp2 = tmp;
		_free (tmp2->name);
		_free (tmp2);
		tmp = tmp->next;
	}
}

int _dgs_channels_readgroup_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	int *count = (int*)p_data;
	
	dgs_channel_t *new_channel = _malloc (sizeof (dgs_channel_t));
	
	if (first_channel == NULL)
	{
		new_channel->prev = NULL;
		new_channel->next = NULL;
		first_channel = new_channel;
		last_channel = new_channel;
	}
	else
	{
		new_channel->prev = last_channel;
		new_channel->next = NULL;
		last_channel->next = new_channel;
		last_channel = new_channel;
	}
	
	last_channel->name = _malloc (strlen (p_fields[1]) + 1);
	strcpy (last_channel->name, p_fields[1]);
	last_channel->id = atoi (p_fields[0]);
	last_channel->nid = atoi (p_fields[2]);
	last_channel->tsid = atoi (p_fields[3]);
	last_channel->sid = atoi (p_fields[4]);
	last_channel->seq = atoi (p_fields[5]);
	last_channel->grp_id = atoi (p_fields[6]);
	last_channel->db_channel = NULL;
	
	*count = *count + 1;
    return 0;
}

void dgs_channels_readgroup (int id)
{
    int ret;
	int count = 0;
    char *errmsg = 0;
	char sql[1024];
	
	if (dgs_getdb () == NULL) return;
	
	sprintf (sql,
			"SELECT	FAVListInfo.ch_id, \
					channelinfo.fullname, \
					channelinfo.org_network_id, \
					channelinfo.ts_id, \
					channelinfo.service_id, \
					FAVListInfo.seq, \
					FAVListInfo.grp_id \
		 	 FROM	FAVListInfo \
		 	 INNER JOIN channelinfo \
		 	 ON FAVListInfo.ch_id = channelinfo.id \
		 	 WHERE FAVListInfo.grp_id = %d AND channelinfo.status == 0 \
		 	 ORDER BY FAVListInfo.seq", id);
	
	ret = sqlite3_exec (dgs_getdb (), sql, _dgs_channels_readgroup_callback, &count, &errmsg);
						
	if (ret == SQLITE_OK)
		channels_count = count;
	else
		log_add ("SQL error: %s", errmsg);
}

dgs_channel_t *dgs_channels_get_first ()
{
	return first_channel;
}

dgs_channel_t *dgs_channels_get_last ()
{
	return last_channel;
}

dgs_channel_t *dgs_channels_get_by_id (int ch_id)
{
	dgs_channel_t *tmp = first_channel;
	
	while (tmp != NULL)
	{
		if (tmp->id == ch_id)
			return tmp;
			
		tmp = tmp->next;
	}
	
	return first_channel;
}

dgs_channel_t *dgs_channels_get_next (dgs_channel_t *channel)
{
	if (channel != NULL)
	{
		if (channel->next != NULL) return channel->next;
		else return first_channel;
	}
	return NULL;
}

dgs_channel_t *dgs_channels_get_prev (dgs_channel_t *channel)
{
	if (channel != NULL)
	{
		if (channel->prev != NULL) return channel->prev;
		else return last_channel;
	}
	return NULL;
}

int dgs_channels_count ()
{
	return channels_count;
}

void dgs_channels_link_to_db ()
{
	dgs_channel_t *tmp = first_channel;
	
	while (tmp != NULL)
	{
		tmp->db_channel = epgdb_channels_get_by_freq (tmp->nid, tmp->tsid, tmp->sid);
		tmp = tmp->next;
	}
}
