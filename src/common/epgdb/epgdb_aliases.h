#ifndef _EPGDB_ALIASES_H_
#define _EPGDB_ALIASES_H_

void epgdb_aliases_clear ();
epgdb_channel_t *epgdb_aliases_add (epgdb_channel_t *channel, unsigned short int nid, unsigned short int tsid, unsigned short int sid);

#endif // _EPGDB_ALIASES_H_
