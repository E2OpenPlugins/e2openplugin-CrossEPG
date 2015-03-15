#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <malloc.h>
#include <stdint.h>

#include "../../common.h"
#include "../core/log.h"
//#include "../core/crc32.h"

#include "huffman.h"
#include "opentv.h"

#include "../epgdb/epgdb_channels.h"
#include "../epgdb/epgdb_titles.h"

#define MAX_TITLE_SIZE		256
#define MAX_SUMMARIE_SIZE	16384
#define MAX_CHANNELS		65536

typedef struct crcs_s
{
	unsigned short size;
	unsigned char *data;
} crcs_t;

static epgdb_channel_t *channels[MAX_CHANNELS];

static unsigned short int ch_count;
static int tit_count;

void opentv_init ()
{
	int i;
	ch_count = 0;
	tit_count = 0;
	for (i=0; i<MAX_CHANNELS; i++)
		channels[i] = NULL;
}

void opentv_cleanup ()
{
	/*
	int i;
	for (i=0; i<MAX_CHANNELS; i++)
		if (channels[i] != NULL)
		{
			epgdb_title_t *title = channels[i]->title_first;
			while (title != NULL)
			{
				epgdb_title_t *tmp = title;
				title = title->next;
				if (tmp->long_description != NULL) _free (tmp->long_description);
				_free (tmp->description);
				_free (tmp);
			}
			_free (channels[i]);
		}
	*/
}

bool opentv_read_channels_bat (unsigned char *data, unsigned int length)
{
	unsigned short int	bouquet_descriptors_length		= ((data[8] & 0x0f) << 8) | data[9];
	unsigned short int	transport_stream_loop_length	= ((data[bouquet_descriptors_length + 10] & 0x0f) << 8) | data[bouquet_descriptors_length + 11];
	unsigned int		offset1							= bouquet_descriptors_length + 12;
	bool				ret								= false;
	
	while (transport_stream_loop_length > 0)
	{
		unsigned short int	tid							= (data[offset1] << 8) | data[offset1 + 1];
		unsigned short int	nid							= (data[offset1 + 2] << 8) | data[offset1 + 3];
		unsigned short int	transport_descriptor_length	= ((data[offset1 + 4] & 0x0f) << 8) | data[offset1 + 5];
		unsigned int		offset2						= offset1 + 6;
		
		offset1							+= (transport_descriptor_length + 6);
		transport_stream_loop_length	-= (transport_descriptor_length + 6);
		
		while (transport_descriptor_length > 0)
		{
			unsigned char	descriptor_tag		= data[offset2];
			unsigned char	descriptor_length	= data[offset2 + 1];
			unsigned int	offset3				= offset2 + 2;
			
			offset2						+= (descriptor_length + 2);
			transport_descriptor_length	-= (descriptor_length + 2);
			
			if (descriptor_tag == 0xb1)
			{
				offset3				+= 2;
				descriptor_length	-= 2;
				while (descriptor_length > 0)
				{
					unsigned short int channel_id;
					//unsigned short int sky_id;
					
					channel_id = (data[offset3 + 3] << 8) | data[offset3 + 4];
					//sky_id = ( data[offset3+5] << 8 ) | data[offset3+6];
					
					if (channels[channel_id] == NULL)
					{
						channels[channel_id] = epgdb_channels_add (nid, tid, (data[offset3] << 8) | data[offset3 + 1]);
						ch_count++;
						ret = true;
					}
					
					offset3				+= 9;
					descriptor_length	-= 9;
				}
			}
		}
	}
	
	return ret;
}

unsigned short opentv_channels_count()
{
	return ch_count;
}

void opentv_read_titles (unsigned char *data, unsigned int length, bool huffman_debug)
{
	epgdb_title_t *title;
	unsigned short int channel_id	= (data[3] << 8) | data[4];
	unsigned short int mjd_time		= (data[8] << 8) | data[9];
	
	if ((channel_id > 0) && (mjd_time > 0))
	{
		unsigned int offset = 10;
		
		while ((offset + 11) < length)
		{
			unsigned short int	event_id;
			unsigned char		description_length;
			unsigned short int	packet_length = ((data[offset + 2] & 0x0f) << 8) | data[offset + 3];
			
			if ((data[offset + 4] != 0xb5) || ((packet_length + offset) > length)) break;
			
			event_id = (data[offset] << 8) | data[offset + 1];
			offset += 4;
			description_length = data[offset + 1] - 7;
			
			if ((offset + 9 + description_length) > length) break;
			
			if (channels[channel_id] != NULL)
			{
				char tmp[256];
				
				/* prepare struct */
				title = _malloc (sizeof (epgdb_title_t));
				title->event_id = event_id;
				title->start_time = ((mjd_time - 40587) * 86400) + ((data[offset + 2] << 9) | (data[offset + 3] << 1));
				title->mjd = mjd_time;
				title->length = ((data[offset + 4] << 9) | (data[offset + 5] << 1));
				title->genre_id = data[offset + 6];
				title->flags = 0;
				//title->genre_sub_id = 0;
				title->iso_639_1 = 'e';		// TODO: load language from provider configuration
				title->iso_639_2 = 'n';
				title->iso_639_3 = 'g';
				title = epgdb_titles_add (channels[channel_id], title);
				
				if (!huffman_decode (data + offset + 9, description_length, tmp, 256, huffman_debug))
					tmp[0] = '\0';
				
				if (huffman_debug)
				{
					char mtime[20];
					struct tm *loctime = localtime ((time_t*)&title->start_time);
					printf ("Nid: %x Tsid: %x Sid: %x\n", channels[channel_id]->nid, channels[channel_id]->tsid, channels[channel_id]->sid);
					strftime (mtime, 20, "%d/%m/%Y %H:%M", loctime);
					printf ("Start time: %s\n", mtime);
				}
				
				epgdb_titles_set_description (title, tmp);
				tit_count++;
			}

			offset += packet_length;
		}
	}
}

void opentv_read_summaries (unsigned char *data, unsigned int length, bool huffman_debug)
{
	if (length < 20) return;
	
	unsigned short int channel_id	= (data[3] << 8) | data[4];
	unsigned short int mjd_time	    = (data[8] << 8) | data[9];
	
	if ((channel_id > 0) && (mjd_time > 0))
	{
		unsigned int offset = 10;
		
		while (offset + 4 < length)
		{
			unsigned short int	event_id;
			int					packet_length = ((data[offset + 2] & 0x0f) << 8) | data[offset + 3];
			int					packet_length2 = packet_length;
			char				buffer[MAX_SUMMARIE_SIZE];
			unsigned short int	buffer_size = 0;
			unsigned int offset2;

			if (packet_length == 0) break;

			event_id = (data[offset] << 8) | data[offset + 1];
			offset += 4;
			offset2 = offset;
			while (packet_length2 > 0)
			{
				unsigned char descriptor_tag = data[offset2];
				unsigned char descriptor_length = data[offset2 + 1];

				offset2 += 2;

				if (descriptor_tag == 0xb9 &&
					MAX_SUMMARIE_SIZE > buffer_size + descriptor_length &&
					offset2 + descriptor_length < length)
				{
					memcpy(&buffer[buffer_size], &data[offset2], descriptor_length);
					buffer_size += descriptor_length;
				}

				packet_length2 -= descriptor_length + 2;
				offset2 += descriptor_length;
			}

			offset += packet_length;

			if (buffer_size > 0 && channels[channel_id] != NULL)
			{
				epgdb_title_t *title = epgdb_titles_get_by_id_and_mjd (channels[channel_id], event_id, mjd_time);
				if (title != NULL)
				{
					char tmp[MAX_SUMMARIE_SIZE * 2];
					if (!huffman_decode (buffer, buffer_size, tmp, MAX_SUMMARIE_SIZE * 2, huffman_debug))
						tmp[0] = '\0';
					
					if (huffman_debug)
					{
						char mtime[20];
						struct tm *loctime = localtime ((time_t*)&title->start_time);
						printf ("Nid: %x Tsid: %x Sid: %x\n", channels[channel_id]->nid, channels[channel_id]->tsid, channels[channel_id]->sid);
						strftime (mtime, 20, "%d/%m/%Y %H:%M", loctime);
						printf ("Start time: %s\n", mtime);
					}
					
					epgdb_titles_set_long_description (title, tmp);
				}
			}
		}
	}
}

epgdb_channel_t *opentv_get_channel (unsigned short int id)
{
	return channels[id];
}
