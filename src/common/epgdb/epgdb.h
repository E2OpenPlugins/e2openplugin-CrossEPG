#ifndef _EPGDB_H_
#define _EPGDB_H_

#include <stdint.h>

#define DB_REVISION	0x05

typedef struct epgdb_title_s
{
	/* same element of epgdb_title_header_t */
	unsigned short int		event_id;
	time_t					start_time;
	unsigned short int		mjd;
	unsigned short int		length;
	unsigned char			genre_id;
	unsigned char			genre_sub_id;
	uint32_t				description_crc;
	unsigned short int		description_length;
	unsigned int			description_seek;
	uint32_t				long_description_crc;
	unsigned short int		long_description_length;
	unsigned int			long_description_seek;
	
	/* other elements */
	bool					changed;
	struct epgdb_title_s	*prev;
	struct epgdb_title_s	*next;
} epgdb_title_t;

typedef struct epgdb_index_s
{
	/* same element of epgdb_index_header_t */
	uint32_t			crc;
	unsigned short int	length;
	unsigned int		seek;

	/* other elements */
	unsigned char			used;
	struct epgdb_index_s	*prev;
	struct epgdb_index_s	*next;
} epgdb_index_t;

typedef struct epgdb_alias_s
{
	unsigned short int		nid;
	unsigned short int		tsid;
	unsigned short int		sid;
} epgdb_alias_t;

typedef struct epgdb_channel_s
{
	/* same element of epgdb_channel_header_t */
	unsigned short int		nid;
	unsigned short int		tsid;
	unsigned short int		sid;

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
