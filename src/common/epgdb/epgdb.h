#ifndef _EPGDB_H_
#define _EPGDB_H_

#include <stdint.h>

#define DB_REVISION	0x07

#define FLAG_UTF8 0x01 // 0000 0001

#define SET_UTF8(x)		(x |= FLAG_UTF8)
#define UNSET_UTF8(x)	(x &= (~FLAG_UTF8))
#define IS_UTF8(x)		(x & FLAG_UTF8)

typedef struct epgdb_title_s
{
	/* same elements of epgdb_title_header_t */
	uint16_t	event_id;
	uint16_t	mjd;
	uint32_t	start_time;
	uint16_t	length;
	uint8_t		genre_id;
	uint8_t		flags;
	uint32_t	description_crc;
	uint32_t	description_seek;
	uint32_t	long_description_crc;
	uint32_t	long_description_seek;
	uint16_t	description_length;
	uint16_t	long_description_length;
	uint8_t		iso_639_1;
	uint8_t		iso_639_2;
	uint8_t		iso_639_3;
	uint8_t		revision;

	/* other elements */
	bool					changed;
	struct epgdb_title_s	*prev;
	struct epgdb_title_s	*next;
} epgdb_title_t;

typedef struct epgdb_index_s
{
	/* same element of epgdb_index_header_t */
	uint32_t	crc;
	uint32_t	seek;
	uint16_t	length;

	/* other elements */
	unsigned char			used;
	struct epgdb_index_s	*prev;
	struct epgdb_index_s	*next;
} epgdb_index_t;

typedef struct epgdb_alias_s
{
	uint16_t	nid;
	uint16_t	tsid;
	uint16_t	sid;
} epgdb_alias_t;

typedef struct epgdb_channel_s
{
	/* same element of epgdb_channel_header_t */
	uint16_t	nid;
	uint16_t	tsid;
	uint16_t	sid;

	/* other elements */
	struct epgdb_channel_s	*prev;
	struct epgdb_channel_s	*next;
	struct epgdb_title_s 	*title_first;
	struct epgdb_title_s 	*title_last;
	//struct epgdb_title_s 	*title_index;
	
	epgdb_alias_t			*aliases;
	unsigned char			aliases_count;
} epgdb_channel_t;

bool epgdb_open (char* db_root);
void epgdb_close ();
bool epgdb_load ();
bool epgdb_save (void(*progress_callback)(int, int));
void epgdb_clean ();
FILE *epgdb_get_fdd ();
time_t epgdb_get_creation_time ();
time_t epgdb_get_update_time ();

#endif // _EPGDB_H_
