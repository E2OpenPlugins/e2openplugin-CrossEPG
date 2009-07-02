#ifndef _DGS_CHANNELS_H_
#define _DGS_CHANNELS_H_

#include "../common/epgdb/epgdb.h"
#include "../common/epgdb/epgdb_titles.h"
#include "../common/epgdb/epgdb_channels.h"

typedef struct dgs_channel_s
{
	int 			 		id;
	char 			 		*name;
	unsigned short int		nid;
	unsigned short int 		tsid;
	unsigned short int 		sid;
	int 			 		seq;
	int						grp_id;
	struct dgs_channel_s	*prev;
	struct dgs_channel_s	*next;
	epgdb_channel_t			*db_channel;
} dgs_channel_t;

void dgs_channels_init ();
void dgs_channels_readgroup (int id);
void dgs_channels_clean ();
dgs_channel_t *dgs_channels_get_first ();
dgs_channel_t *dgs_channels_get_last ();
dgs_channel_t *dgs_channels_get_by_id (int ch_id);
dgs_channel_t *dgs_channels_get_next (dgs_channel_t *channel);
dgs_channel_t *dgs_channels_get_prev (dgs_channel_t *channel);
int dgs_channels_count ();
void dgs_channels_link_to_db ();

#endif // _DGS_CHANNELS_H_
