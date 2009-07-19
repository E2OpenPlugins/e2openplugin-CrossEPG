#ifndef _OPENTV_H_
#define _OPENTV_H_

#include "../epgdb/epgdb.h"

void opentv_init ();
void opentv_cleanup ();
bool opentv_read_channels_bat (unsigned char *data, unsigned int length);
unsigned short opentv_channels_count ();
void opentv_read_titles (unsigned char *data, unsigned int length, bool huffman_debug);
void opentv_read_summaries (unsigned char *data, unsigned int length, bool huffman_debug);
epgdb_channel_t *opentv_get_channel (unsigned short int id);
//epgdb_channel_t *opentv_get_first_channel ();
//int opentv_channels_count();
//int opentv_titles_count(unsigned short int id);

#endif // _OPENTV_H_
