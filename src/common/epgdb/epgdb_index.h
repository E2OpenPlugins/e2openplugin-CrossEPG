#ifndef _EPGDB_INDEX_H_
#define _EPGDB_INDEX_H_

int epgdb_index_count ();
int epgdb_index_empties_count ();
epgdb_index_t *epgdb_index_get_first (unsigned short int key);
epgdb_index_t *epgdb_index_empties_get_first ();
void epgdb_index_empties_set_first (epgdb_index_t *index);
void epgdb_index_empties_set_last (epgdb_index_t *index);
void epgdb_index_empties_add (epgdb_index_t *index);
void epgdb_index_init ();
void epgdb_index_clean ();
epgdb_index_t *epgdb_index_get_by_crc_length (uint32_t crc, unsigned short int length);
epgdb_index_t *epgdb_index_add (uint32_t crc, unsigned short int length, bool *added);
void epgdb_index_mark_all_as_unused ();
void epgdb_index_mark_as_used (uint32_t crc, unsigned short int length);
void epgdb_index_empty_unused ();

#endif // _EPGDB_INDEX_H_
