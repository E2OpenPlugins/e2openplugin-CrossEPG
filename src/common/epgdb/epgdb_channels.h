#ifndef _EPGDB_CHANNELS_H_
#define _EPGDB_CHANNELS_H_

int epgdb_channels_count ();
epgdb_channel_t *epgdb_channels_get_first ();
void epgdb_channels_set_first (epgdb_channel_t *channel);
void epgdb_channels_set_last (epgdb_channel_t *channel);
epgdb_channel_t *epgdb_channels_get_by_freq (unsigned short int nid, unsigned short int tsid, unsigned short int sid);
epgdb_channel_t *epgdb_channels_add (unsigned short int nid, unsigned short int tsid, unsigned short int sid);

#endif // _EPGDB_CHANNELS_H_
