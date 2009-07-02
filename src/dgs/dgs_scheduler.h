#ifndef _DGS_SCHEDULER_H_
#define _DGS_SCHEDULER_H_

typedef struct dgs_scheduler_s
{
	int 			 		id;
	int						ch_id;
	char 			 		*name;
	int						mode;
	time_t					start_time;
	int						length;
	bool					is_recording;
	struct dgs_scheduler_s	*prev;
	struct dgs_scheduler_s	*next;
} dgs_scheduler_t;

void dgs_scheduler_init ();
bool dgs_scheduler_read ();
void dgs_scheduler_clean ();
dgs_scheduler_t *dgs_scheduler_get_first ();
dgs_scheduler_t *dgs_scheduler_get_last ();
dgs_scheduler_t *dgs_scheduler_get_next (dgs_scheduler_t *scheduler);
dgs_scheduler_t *dgs_scheduler_get_prev (dgs_scheduler_t *scheduler);
dgs_scheduler_t *dgs_scheduler_get_by_channel_and_title (dgs_channel_t *channel, epgdb_title_t *title);
dgs_scheduler_t *dgs_scheduler_get_by_channelid_and_title (int channel_id, epgdb_title_t *title);
int dgs_scheduler_count ();

#endif // _DGS_SCHEDULER_H_
