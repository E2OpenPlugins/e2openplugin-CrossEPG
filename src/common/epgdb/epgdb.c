#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "../../common.h"

#include "epgdb.h"
#include "epgdb_index.h"
#include "epgdb_channels.h"
#include "epgdb_titles.h"

#define MAGIC_HEADERS	"_xEPG_HEADERS"
#define FAKE_HEADERS	"__NO__HEADERS"
#define MAGIC_ALIASES	"_xEPG_ALIASES"
#define FAKE_ALIASES	"__NO__ALIASES"
#define MAGIC_INDEXES	"_xEPG_INDEXES"
#define FAKE_INDEXES	"__NO__INDEXES"

typedef struct epgdb_channel_header_s
{
	unsigned short int		nid;
	unsigned short int 		tsid;
	unsigned short int		sid;
} epgdb_channel_header_t;

typedef struct epgdb_title_header_s
{
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
} epgdb_title_header_t;

typedef struct epgdb_index_header_s
{
	uint32_t			crc;
	unsigned short int	length;
	unsigned int		seek;
} epgdb_index_header_t;

static FILE *fd_h = NULL;
static FILE *fd_d = NULL;
static FILE *fd_i = NULL;
static FILE *fd_a = NULL;
static char header_filename[256];
static char descriptor_filename[256];
static char index_filename[256];
static char aliases_filename[256];

static time_t db_creation_time = 0;
static time_t db_update_time = 0;

FILE *epgdb_get_fdd () { return fd_d; }
time_t epgdb_get_creation_time () { return db_creation_time; }
time_t epgdb_get_update_time () { return db_update_time; }

bool epgdb_open (char* db_root)
{
	db_creation_time = db_update_time = time (NULL);
	sprintf (header_filename, "%s/crossepg.headers.db", db_root);
	sprintf (descriptor_filename, "%s/crossepg.descriptors.db", db_root);
	sprintf (index_filename, "%s/crossepg.indexes.db", db_root);
	sprintf (aliases_filename, "%s/crossepg.aliases.db", db_root);
	
	fd_h = fopen (header_filename, "r+");
	if (fd_h == NULL) fd_h = fopen (header_filename, "w+");
	if (fd_h == NULL) return false;
	
	fd_d = fopen (descriptor_filename, "r+");
	if (fd_d == NULL) fd_d = fopen (descriptor_filename, "w+");
	if (fd_d == NULL)
	{
		fclose (fd_h);
		fd_h = NULL;
		return false;
	}
	
	fd_i = fopen (index_filename, "r+");
	if (fd_i == NULL) fd_i = fopen (index_filename, "w+");
	if (fd_i == NULL)
	{
		fclose (fd_h);
		fclose (fd_d);
		fd_h = NULL;
		fd_d = NULL;
		return false;
	}

	fd_a = fopen (aliases_filename, "r+");
	if (fd_a == NULL) fd_a = fopen (aliases_filename, "w+");
	if (fd_a == NULL)
	{
		fclose (fd_h);
		fclose (fd_d);
		fclose (fd_i);
		fd_h = NULL;
		fd_d = NULL;
		fd_i = NULL;
		return false;
	}
	
	return true;
}

void epgdb_close ()
{
	if (fd_d != NULL) fclose (fd_d);
	if (fd_h != NULL) fclose (fd_h);
	if (fd_i != NULL) fclose (fd_i);
	if (fd_a != NULL) fclose (fd_a);
	fd_d = NULL;
	fd_h = NULL;
	fd_i = NULL;
	fd_a = NULL;
}

bool epgdb_save (void(*progress_callback)(int, int))
{
	int channels_count, titles_count, indexes_count, aliases_groups_count, i;
	unsigned char revision;
	epgdb_channel_t *channel;
	int progress_count = 0;
	int progress_max = epgdb_channels_count () + epgdb_index_count ();
	
	if ((fd_d == NULL) || (fd_h == NULL) || (fd_i == NULL) || (fd_a == NULL)) return false;
	fsync (fileno (fd_d));
	
	epgdb_index_mark_all_as_unused ();
	
	/* close and reopen headers for write */
	fclose (fd_h);
	fd_h = fopen (header_filename, "w");

	fclose (fd_i);
	fd_i = fopen (index_filename, "w");
	
	fclose (fd_a);
	fd_a = fopen (aliases_filename, "w");
	
	/* write headers */
	revision = DB_REVISION;
	fwrite (FAKE_HEADERS, strlen (FAKE_HEADERS), 1, fd_h);
	fwrite (&revision, sizeof (unsigned char), 1, fd_h);
	
	db_update_time = time (NULL);
	fwrite (&db_creation_time, sizeof (time_t), 1, fd_h);
	fwrite (&db_update_time, sizeof (time_t), 1, fd_h);
	
	channels_count = 0;
	fwrite (&channels_count, sizeof (int), 1, fd_h); // write the exact value at end
	
	channel = epgdb_channels_get_first ();
	while (channel != NULL)
	{
		titles_count = epgdb_titles_count (channel);
		if (titles_count > 0)
		{
			epgdb_title_t *title = channel->title_first;
			fwrite (channel, sizeof (epgdb_channel_header_t), 1, fd_h);
			fwrite (&titles_count, sizeof (int), 1, fd_h);
			
			while (title != NULL)
			{
				fwrite (title, sizeof (epgdb_title_header_t), 1, fd_h);
				epgdb_index_mark_as_used (title->description_crc, title->description_length);
				epgdb_index_mark_as_used (title->long_description_crc, title->long_description_length);
				title = title->next;
			}
			channels_count++;
		}
		channel = channel->next;
		progress_count++;
		if (progress_callback != NULL)
			progress_callback (progress_count, progress_max);
	}
	fseek (fd_h, strlen (MAGIC_HEADERS) + sizeof (unsigned char) + (sizeof (time_t) * 2), SEEK_SET);
	fwrite (&channels_count, sizeof (int), 1, fd_h);
	fflush (fd_h);
	fsync (fileno (fd_h));
	fseek (fd_h, 0, SEEK_SET);
	fwrite (MAGIC_HEADERS, strlen (MAGIC_HEADERS), 1, fd_h);
	fflush (fd_h);
	fclose (fd_h);
	fd_h = fopen (header_filename, "r+");

	/* write indexes */
	fwrite (FAKE_INDEXES, strlen (FAKE_INDEXES), 1, fd_i);
	fwrite (&revision, sizeof (unsigned char), 1, fd_i);
	
	void epgdb_index_empty_unused ();
	
	indexes_count = epgdb_index_count ();
	fwrite (&indexes_count, sizeof (int), 1, fd_i);
	
	for (i=0; i<65536; i++)
	{
		epgdb_index_t *index = epgdb_index_get_first (i);
		while (index != NULL)
		{
			fwrite (index, sizeof (epgdb_index_header_t), 1, fd_i);
			index = index->next;
			
			progress_count++;
			if (progress_callback != NULL)
				progress_callback (progress_count, progress_max);
		}
	}

	/* empty indexes */
	indexes_count = epgdb_index_empties_count ();
	fwrite (&indexes_count, sizeof (int), 1, fd_i);
	epgdb_index_t *index = epgdb_index_empties_get_first ();
	while (index != NULL)
	{
		fwrite (index, sizeof (epgdb_index_header_t), 1, fd_i);
		index = index->next;
	}
	
	fflush (fd_i);
	fsync (fileno (fd_i));
	fseek (fd_i, 0, SEEK_SET);
	fwrite (MAGIC_INDEXES, strlen (MAGIC_INDEXES), 1, fd_i);
	fflush (fd_i);
	fclose (fd_i);
	fd_i = fopen (index_filename, "r+");
	
	/* write aliases */
	fwrite (FAKE_ALIASES, strlen (FAKE_ALIASES), 1, fd_a);
	fwrite (&revision, sizeof (unsigned char), 1, fd_a);
	
	aliases_groups_count = 0;
	fwrite (&aliases_groups_count, sizeof (int), 1, fd_a); // write the exact value at end
	
	channel = epgdb_channels_get_first ();
	while (channel != NULL)
	{
		titles_count = epgdb_titles_count (channel);
		if ((channel->aliases_count > 0) && (titles_count > 0))
		{
			int i;
			fwrite (channel, sizeof (epgdb_channel_header_t), 1, fd_a);
			fwrite (&channel->aliases_count, sizeof (unsigned char), 1, fd_a);
			for (i=0; i<channel->aliases_count; i++)
				fwrite (&channel->aliases[i], sizeof (epgdb_alias_t), 1, fd_a);
			
			aliases_groups_count++;
		}
		channel = channel->next;
	}
	fseek (fd_a, strlen (MAGIC_ALIASES) + sizeof (unsigned char), SEEK_SET);
	fwrite (&aliases_groups_count, sizeof (int), 1, fd_a);
	fflush (fd_a);
	fsync (fileno (fd_a));
	fseek (fd_a, 0, SEEK_SET);
	fwrite (MAGIC_ALIASES, strlen (MAGIC_ALIASES), 1, fd_a);
	fflush (fd_a);
	fclose (fd_a);
	fd_a = fopen (aliases_filename, "r+");
	return true;
}

bool epgdb_load ()
{
	char tmp[256];
	unsigned char revision;
	int channels_count, i, j, aliases_groups_count, indexes_count;
	time_t min = time (NULL) - (60*60*6);
	
	epgdb_index_init ();
	
	fseek (fd_h, 0, SEEK_SET);
	fseek (fd_a, 0, SEEK_SET);
	fseek (fd_i, 0, SEEK_SET);
	fseek (fd_d, 0, SEEK_SET);
	
	/* read headers */
	fread (tmp, strlen (MAGIC_HEADERS), 1, fd_h);
	if (memcmp (tmp, MAGIC_HEADERS, strlen (MAGIC_HEADERS)) != 0) return false;
	fread (&revision, sizeof (unsigned char), 1, fd_h);
	if (revision != DB_REVISION) return false;
	
	fread (&db_creation_time, sizeof (time_t), 1, fd_h);
	fread (&db_update_time, sizeof (time_t), 1, fd_h);
	
	fread (&channels_count, sizeof (int), 1, fd_h);
	for (i=0; i<channels_count; i++)
	{
		int titles_count;
		epgdb_channel_t *tmp;
		epgdb_channel_t *channel = _malloc (sizeof (epgdb_channel_t));
		fread (channel, sizeof (epgdb_channel_header_t), 1, fd_h);
		channel->title_first = NULL;
		channel->title_last = NULL;
		channel->next = NULL;
		channel->prev = NULL;
		channel->aliases = NULL;
		channel->aliases_count = 0;
		
		tmp = epgdb_channels_get_first();
		if (tmp == NULL) epgdb_channels_set_first (channel);
		else
		{
			while (tmp->next != NULL) tmp = tmp->next;
			
			tmp->next = channel;
			channel->prev = tmp;
			epgdb_channels_set_last (channel);
		}
		
		fread (&titles_count, sizeof (int), 1, fd_h);
		
		for (j=0; j<titles_count; j++)
		{
			epgdb_title_t *title = _malloc (sizeof (epgdb_title_t));
			fread (title, sizeof (epgdb_title_header_t), 1, fd_h);
			if (title->start_time > min)
			{
				title->prev = NULL;
				title->next = NULL;
				title->changed = false;
				
				if (channel->title_last == NULL)
				{
					channel->title_first = title;
					channel->title_last = title;
				}
				else
				{
					channel->title_last->next = title;
					title->prev = channel->title_last;
					channel->title_last = title;
				}
			}
			else
				_free (title);
		}
	}
	
	/* read indexes */
	fread (tmp, strlen (MAGIC_INDEXES), 1, fd_i);
	if (memcmp (tmp, MAGIC_INDEXES, strlen (MAGIC_INDEXES)) != 0) return false;
	fread (&revision, sizeof (unsigned char), 1, fd_i);
	if (revision != DB_REVISION) return false;
	
	fread (&indexes_count, sizeof (int), 1, fd_i);
	for (i=0; i<indexes_count; i++)
	{
		bool added;
		epgdb_index_t tindex;
		fread (&tindex, sizeof (epgdb_index_header_t), 1, fd_i);
		
		epgdb_index_t *index = epgdb_index_add (tindex.crc, tindex.length, &added);
		if (added) index->seek = tindex.seek;
	}
	/* empty indexes */
	indexes_count = 0;
	fread (&indexes_count, sizeof (int), 1, fd_i);
	for (i=0; i<indexes_count; i++)
	{
		epgdb_index_t *index = _malloc (sizeof (epgdb_index_t));
		
		fread (index, sizeof (epgdb_index_header_t), 1, fd_i);
		epgdb_index_empties_add (index);
	}
	
	/* read aliases */
	fread (tmp, strlen (MAGIC_ALIASES), 1, fd_a);
	if (memcmp (tmp, MAGIC_ALIASES, strlen (MAGIC_ALIASES)) != 0) return false;
	fread (&revision, sizeof (unsigned char), 1, fd_a);
	if (revision != DB_REVISION) return false;
	
	fread (&aliases_groups_count, sizeof (int), 1, fd_a);
	for (i=0; i<aliases_groups_count; i++)
	{
		unsigned char aliases_count;
		epgdb_channel_t *tmp;
		epgdb_channel_t *channel = _malloc (sizeof (epgdb_channel_t));
		fread (channel, sizeof (epgdb_channel_header_t), 1, fd_a);
		tmp = epgdb_channels_get_by_freq (channel->nid, channel->tsid, channel->sid);
		
		fread (&aliases_count, sizeof (unsigned char), 1, fd_a);
		if (tmp != NULL)
		{
			tmp->aliases_count = aliases_count;
			if (tmp->aliases != NULL) _free (tmp->aliases);
			tmp->aliases = _malloc (sizeof (epgdb_alias_t)*aliases_count);
		}
		
		for (j=0; j<aliases_count; j++)
		{
			epgdb_alias_t *alias = _malloc (sizeof (epgdb_alias_t));
			fread (alias, sizeof (epgdb_alias_t), 1, fd_a);
			
			if (tmp != NULL)
			{
				tmp->aliases[j].nid = alias->nid;
				tmp->aliases[j].tsid = alias->tsid;
				tmp->aliases[j].sid = alias->sid;
			}
			_free (alias);
		}
		_free (channel);
	}
	return true;
}

void epgdb_clean ()
{
	epgdb_channel_t *channel = epgdb_channels_get_first ();
	
	while (channel != NULL)
	{
		epgdb_channel_t *tmp = channel;
		channel = channel->next;
		epgdb_title_t *title = tmp->title_first;
		
		while (title != NULL)
		{
			epgdb_title_t *tmp2 = title;
			title = title->next;
			_free (tmp2);
		}
		
		if (tmp->aliases != NULL) _free (tmp->aliases);
		
		_free (tmp);
	}
	epgdb_index_clean ();
}
