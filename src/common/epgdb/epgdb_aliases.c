#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "../../common.h"

#include "epgdb.h"
#include "epgdb_channels.h"
#include "epgdb_aliases.h"

void epgdb_aliases_clear ()
{
	epgdb_channel_t *tmp = epgdb_channels_get_first ();
	
	while (tmp != NULL)
	{
		if (tmp->aliases != NULL) _free (tmp->aliases);
		tmp->aliases = NULL;
		tmp->aliases_count = 0;
		
		tmp = tmp->next;
	}
}

epgdb_channel_t *epgdb_aliases_add (epgdb_channel_t *channel, unsigned short int nid, unsigned short int tsid, unsigned short int sid)
{
	epgdb_channel_t *tmp = epgdb_channels_get_by_freq (nid, tsid, sid);
	
	if (tmp == NULL)
	{
		if (channel->aliases == NULL)
		{
			channel->aliases = _malloc (sizeof (epgdb_alias_t));
			channel->aliases[0].nid = nid;
			channel->aliases[0].tsid = tsid;
			channel->aliases[0].sid = sid;
			channel->aliases_count = 1;
		}
		else
		{
			int i;
			epgdb_alias_t *tmp2 = _malloc (sizeof (epgdb_alias_t) * (channel->aliases_count + 1));
			for (i=0; i<channel->aliases_count; i++)
			{
				tmp2[i].nid = channel->aliases[i].nid;
				tmp2[i].tsid = channel->aliases[i].tsid;
				tmp2[i].sid = channel->aliases[i].sid;
			}
			tmp2[channel->aliases_count].nid = nid;
			tmp2[channel->aliases_count].tsid = tsid;
			tmp2[channel->aliases_count].sid = sid;
			_free (channel->aliases);
			channel->aliases = tmp2;
			channel->aliases_count++;
		}
		
		return channel;
	}
	else if (tmp == channel)
	{
		return channel;
	}
	
	return NULL;
}
