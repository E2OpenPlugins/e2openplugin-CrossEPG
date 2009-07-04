#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct scheduler_s
{
	int					channel_id;
	time_t				start_time;
	int					length;
	int					type;
	int					name_length;
	char				*name;
	struct scheduler_s	*next;
	struct scheduler_s	*prev;
} scheduler_t;

#define SCHEDULER_T_SIZE ((sizeof (int)*4)+sizeof(time_t))

scheduler_t *scheduler_get_first ();
void scheduler_init ();
void scheduler_clean ();
bool scheduler_load (char *dbroot);
bool scheduler_save (char *dbroot);
void scheduler_del (int channel_id, time_t start_time, int length, int type, bool with_dgs);
int scheduler_add (int channel_id, time_t start_time, int length, int type, char *name, bool with_dgs);
scheduler_t *scheduler_get_by_channel_and_title (dgs_channel_t *channel, epgdb_title_t *title);
void scheduler_link ();

#endif // _SCHEDULER_H_
