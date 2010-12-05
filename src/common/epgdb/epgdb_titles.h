#ifndef _EPGDB_TITLES_H_
#define _EPGDB_TITLES_H_

epgdb_title_t *epgdb_title_alloc ();
void epgdb_title_free (epgdb_title_t *title);
int epgdb_calculate_mjd (time_t value);
int epgdb_titles_count (epgdb_channel_t *channel);
char *epgdb_read_description (epgdb_title_t *title);
char *epgdb_read_long_description (epgdb_title_t *title);
epgdb_title_t *epgdb_titles_set_long_description (epgdb_title_t *title, char *description);
epgdb_title_t *epgdb_titles_set_description (epgdb_title_t *title, char *description);
epgdb_title_t *epgdb_titles_set_long_description_unicode (epgdb_title_t *title, wchar_t *description);
epgdb_title_t *epgdb_titles_set_description_unicode (epgdb_title_t *title, wchar_t *description);
epgdb_title_t *epgdb_titles_get_by_id_and_mjd (epgdb_channel_t *channel, unsigned short int event_id, unsigned short int mjd_time);
epgdb_title_t *epgdb_titles_get_by_time (epgdb_channel_t *channel, time_t ttime);
void epgdb_titles_delete_in_range (epgdb_channel_t *channel, time_t start_time, unsigned short int length);
epgdb_title_t *epgdb_titles_add (epgdb_channel_t *channel, epgdb_title_t *title);

#endif // _EPGDB_TITLES_H_
