#ifndef _EPGDB_SEARCH_H_
#define _EPGDB_SEARCH_H_

typedef struct epgdb_search_res_s
{
	epgdb_title_t	**titles;
	epgdb_channel_t **channels;
	int				count;
} epgdb_search_res_t;

void epgdb_search_free (epgdb_search_res_t *value);
epgdb_search_res_t *epgdb_search_by_name (char *name, int max_results);
epgdb_title_t *epgdb_search_by_name_freq_time (char *name, int nid, int tsid, int sid, time_t from_time, time_t to_time);

#endif // _EPGDB_SEARCH_H_
