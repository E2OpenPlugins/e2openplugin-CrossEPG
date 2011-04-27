#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>

#include "../common.h"

#include "../common/core/log.h"
#include "../common/core/crc32.h"
#include "../common/core/interactive.h"

#include "../common/epgdb/epgdb.h"
#include "../common/epgdb/epgdb_channels.h"
#include "../common/epgdb/epgdb_titles.h"

#include "enigma2_common.h"
#include "enigma2_hash.h"
#include "enigma2_lamedb.h"

static volatile bool stop = false;
static volatile bool exec = false;
static volatile bool quit = false;
static volatile bool timeout_enable = true;
pthread_mutex_t mutex;
int timeout = 0;

char lamedb[256]; // = DEFAULT_LAMEDB;
char epgdat[256]; // = DEFAULT_EPG_DAT;

typedef struct sdesc_s
{
	unsigned char *data;
	unsigned char size;
} sdesc_t;

typedef struct ldesc_s
{
	unsigned char *data[16];
	unsigned char size[16];
	char count;
} ldesc_t;

static sdesc_t* short_desc (char *value, unsigned char iso639_1, unsigned char iso639_2, unsigned char iso639_3, bool isutf8)
{
	sdesc_t *ret = _malloc (sizeof (sdesc_t));
	ret->size = strlen (value);
	if (ret->size > 245)
		ret->size = 245;
	ret->data = _malloc (ret->size + 8);
	ret->data[0] = 0x4d;
	if (isutf8)
		ret->data[1] = ret->size + 6;
	else
		ret->data[1] = ret->size + 5;
	ret->data[2] = iso639_1;
	ret->data[3] = iso639_2;
	ret->data[4] = iso639_3;
	if (isutf8)
	{
		ret->data[5] = ret->size + 1;
		ret->data[6] = 0x15;
		memcpy (ret->data+7, value, ret->size);
		ret->data[ret->size+7] = 0x00;
		ret->size += 8;
	}
	else
	{
		ret->data[5] = ret->size;
		memcpy (ret->data+6, value, ret->size);
		ret->data[ret->size+6] = 0x00;
		ret->size += 7;
	}
	return ret;
}

static ldesc_t* long_desc (char *value, unsigned char iso639_1, unsigned char iso639_2, unsigned char iso639_3, bool isutf8)
{
	int i;
	ldesc_t* ret = _malloc (sizeof (ldesc_t));
	int maxchars = 246;
	if (isutf8)
		maxchars = 245;

	ret->count = strlen (value) / maxchars;

	for (i=0; i<ret->count; i++)
		ret->size[i] = 254;
		
	if ((strlen (value) % maxchars) > 0)
	{
		ret->size[(int)ret->count] = (strlen (value) % maxchars) + 8;
		if (isutf8)
			ret->size[(int)ret->count]++;
		ret->count++;
	}
	
	for (i=0; i<ret->count; i++)
	{
		ret->data[i] = _malloc (ret->size[i]);
		if (isutf8)
			memcpy (ret->data[i]+9, value+(i*245), ret->size[i]-9);
		else
			memcpy (ret->data[i]+8, value+(i*246), ret->size[i]-8);
		ret->data[i][0] = 0x4e;
		ret->data[i][1] = ret->size[i] - 2;
		ret->data[i][2] = (i << 4) + (ret->count - 1);
		ret->data[i][3] = iso639_1;
		ret->data[i][4] = iso639_2;
		ret->data[i][5] = iso639_3;
		ret->data[i][6] = 0x00;
		//ret->data[i][7] = ret->size[i] + 1 - 9;
		ret->data[i][7] = ret->size[i] - 8;
		if (isutf8)
			ret->data[i][8] = 0x15;
	}
	
	return ret;
}

void progress (int value, int max)
{
	static int last = 0;
	int now = (value*100)/max;
	if (now != last)
	{
		interactive_send_int (ACTION_PROGRESS, now);
		last = now;
	}
}

int events_count = 0;

static void write_titles (epgdb_channel_t *channel, FILE *fd)
{
	epgdb_title_t *title = channel->title_first;
	while (title != NULL)
	{
		int i;
		char *buf;
		int crcs_count = 1;
		uint32_t crcs[17];
		char length;
		struct tm start_time;
		
		/* description */
		char *description = epgdb_read_description (title);
		//if (strlen (description) > 245) description[245] = '\0';
		if (strlen (description) > 246) description[246] = '\0';
		gmtime_r ((time_t*)&title->start_time, &start_time);
		sdesc_t *sdesc = short_desc (description, title->iso_639_1, title->iso_639_2, title->iso_639_3, IS_UTF8(title->flags));
		
		crcs[0] = crc32 (sdesc->data, sdesc->size);
		if (!enigma2_hash_add (crcs[0], sdesc->data, sdesc->size)) _free (sdesc->data);
		_free (sdesc);
		_free (description);
		
		/* long description */
		char *ldescription = epgdb_read_long_description (title);
		if (strlen (ldescription) > 0)
		{
			if (IS_UTF8(title->flags))
			{
				if (strlen (ldescription) > (245*16)) ldescription[245*16] = '\0';
			}
			else
			{
				if (strlen (ldescription) > (246*16)) ldescription[246*16] = '\0';
			}
			ldesc_t *ldesc = long_desc (ldescription, title->iso_639_1, title->iso_639_2, title->iso_639_3, IS_UTF8(title->flags));
			
			for (i=0; i<ldesc->count; i++)
			{
				crcs[i+1] = crc32 (ldesc->data[i], ldesc->size[i]);
				if (!enigma2_hash_add (crcs[i+1], ldesc->data[i], ldesc->size[i])) _free (ldesc->data[i]);
				crcs_count++;
			}
			
			_free (ldesc);
		}
		
		_free (ldescription);
		
		events_count++;
		uint16_t event_id = events_count;
		uint16_t start_mjd = title->mjd;
		length = 10 + (crcs_count * 4);
		buf = _malloc (length + 2);
		buf[0] = 0x01;
		buf[1] = length ;
		buf[2] = (event_id >> 8) & 0xff;
		buf[3] = event_id & 0xff;
		buf[4] = (start_mjd >> 8) & 0xff;
		buf[5] = start_mjd & 0xff;
		buf[6] = toBCD (start_time.tm_hour);
		buf[7] = toBCD (start_time.tm_min);
		buf[8] = toBCD (start_time.tm_sec);
		buf[9] = toBCD (title->length / (60*60));
		buf[10] = toBCD ((title->length / 60) % 60);
		buf[11] = toBCD (title->length % 60);
		for (i=0; i<crcs_count; i++)
		{
			memcpy (buf+12+(i*4), &crcs[i], 4);
		}
		
		fwrite (buf, length+2, 1, fd);
		_free (buf);
		
		title = title->next;
		//count++;
		if (stop) return;
	}
}

static void write_epgdat ()
{
	int i;
	//int count = 1;
	int ccount = 0;
	int progress_max = epgdb_channels_count ();
	int progress_now = 0;
	FILE *fd;
	
	events_count = 0;
	
	interactive_send (ACTION_START);
	interactive_send_text (ACTION_PROGRESS, "ON");
	
	enigma2_hash_init ();
	
	fd = fopen(epgdat, "w");
	if (fd == NULL)
	{
		log_add ("Cannot open epg.dat");
		interactive_send_text (ACTION_ERROR, "cannot open epg.dat");
		goto write_end;
	}
	log_add ("EPG.DAT opened");
	
	if (!enigma2_lamedb_read (lamedb))
	{
		log_add ("Error reading lamedb");
		//interactive_send_text (ACTION_ERROR, "error reading lamedb");
		interactive_send_text (ACTION_ERROR, lamedb);
		goto write_end;
	}
	
	unsigned int magic = 0x98765432;
	fwrite (&magic, sizeof (int), 1, fd);
	const char *header = "UNFINISHED_V7";
	fwrite (header, 13, 1, fd);
	fwrite (&ccount, sizeof (int), 1, fd); 		// write the exact number at the end
		
	epgdb_channel_t *channel = epgdb_channels_get_first ();
	
	log_add ("Writing events...");
	while (channel != NULL)
	{
		int titles_count = epgdb_titles_count (channel);
		if (titles_count > 0)
		{
			if (enigma2_lamedb_exist (channel->nid, channel->tsid, channel->sid))
			{
				int nid = channel->nid;
				int tsid = channel->tsid;
				int sid = channel->sid;
				fwrite (&sid, sizeof (int), 1, fd);
				fwrite (&nid, sizeof (int), 1, fd);
				fwrite (&tsid, sizeof (int), 1, fd);
				fwrite (&titles_count, sizeof (int), 1, fd);
				
				write_titles (channel, fd);
				
				if (stop) goto write_end;
				ccount++;
			}
			
			for (i = 0; i < channel->aliases_count; i++)
			{
				if (enigma2_lamedb_exist (channel->aliases[i].nid, channel->aliases[i].tsid, channel->aliases[i].sid))
				{
					int nid = channel->aliases[i].nid;
					int tsid = channel->aliases[i].tsid;
					int sid = channel->aliases[i].sid;
					fwrite (&sid, sizeof (int), 1, fd);
					fwrite (&nid, sizeof (int), 1, fd);
					fwrite (&tsid, sizeof (int), 1, fd);
					fwrite (&titles_count, sizeof (int), 1, fd);
					
					write_titles (channel, fd);
					
					if (stop) goto write_end;
					ccount++;
				}
			}
		}
		
		progress_now++;
		progress (progress_now, progress_max);
		
		channel = channel->next;
		if (stop) goto write_end;
	}
	
	log_add ("Sorting hashes...");
	enigma2_hash_sort ();
	log_add ("Writing descriptors...");
	int hcount = enigma2_hash_count ();
	fwrite (&hcount, sizeof (int), 1, fd);
	for (i=0; i<65536;i++)
	{
		if (stop) goto write_end;
		enigma2_hash_t *hash = enigma2_hash_get_first(i);
		while (hash != NULL)
		{
			if (stop) goto write_end;
			fwrite (&hash->hash, sizeof (uint32_t), 1, fd);
			fwrite (&hash->use_count, sizeof (int), 1, fd);
			fwrite (hash->data, hash->size, 1, fd);
			hash = hash->next;
		}
	}
	
	/* write the exact number of channels */
	fseek (fd, sizeof (int)+13, SEEK_SET);
	fwrite (&ccount, sizeof (int), 1, fd);
	
	/* sync data on disk and mark epg.dat as a good epg */
	fsync (fileno (fd));
	fseek (fd, sizeof (int), SEEK_SET);
	fwrite ("ENIGMA_EPG_V7", 13, 1, fd);

write_end:
	if (fd) fclose (fd);
	log_add ("EPG.DAT closed");	
	
	enigma2_hash_clean ();
	interactive_send_text (ACTION_PROGRESS, "OFF");
	interactive_send (ACTION_END);
	exec = false;
}

char *format_text (char *value)
{
	int i, z = 0;
	int count = 0;
	for (i=0; i<strlen(value); i++)
		if (value[i] == '\n') count++;
	
	char *ret = _malloc (strlen (value) + count + 1);
	
	for (i=0; i<strlen(value); i++)
	{
		if (value[i] == '\n')
		{
			ret[z] = '\\';
			z++;
			ret[z] = 'n';
			z++;
		}
		else
		{
			ret[z] = value[i];
			z++;
		}
	}
	ret[z] = '\0';
	return ret;
}

void write_text ()
{
	int progress_max = epgdb_channels_count ();
	int progress_now = 0;

	if (!enigma2_lamedb_read (lamedb))
	{
		log_add ("Error reading lamedb");
		//interactive_send_text (ACTION_ERROR, "error reading lamedb");
		interactive_send_text (ACTION_ERROR, lamedb);
		goto write_end;
	}

	interactive_send (ACTION_START);
	interactive_send_text (ACTION_PROGRESS, "ON");

	epgdb_channel_t *channel = epgdb_channels_get_first ();

	while (channel != NULL)
	{
		int namespace = enigma2_lamedb_get_namespace (channel->nid, channel->tsid, channel->sid);
		if (namespace != -1)
		{
			printf ("CHANNEL 1:0:1:%X:%X:%X:%X:0:0:0:\n", channel->sid, channel->tsid, channel->nid, namespace);
			epgdb_title_t *title = channel->title_first;
			while (title != NULL)
			{
				printf ("STARTTIME %u\n", title->start_time);
				printf ("LENGTH %d\n", title->length);
				char *description = epgdb_read_description (title);
				char *description2 = format_text (description);
				char *ldescription = epgdb_read_long_description (title);
				char *ldescription2 = format_text (ldescription);
				printf ("NAME %s\n", description2);
				printf ("DESCRIPTION %s\n", ldescription2);
				_free (description);
				_free (description2);
				_free (ldescription);
				_free (ldescription2);
				title = title->next;
				if (stop) goto write_end;
			}
		}
		progress_now++;
		progress (progress_now, progress_max);
	
		channel = channel->next;
		if (stop) goto write_end;
	}

write_end:
	interactive_send_text (ACTION_PROGRESS, "OFF");
	interactive_send (ACTION_END);
	exec = false;
}

void *text (void *args)
{
	write_text ();
	return NULL;
}

void *convert (void *args)
{
	write_epgdat ();
	return NULL;
}

void* interactive (void *args)
{
	char buffer[4096], byte;
	bool run = true;
	pthread_t thread;
	exec = false;
	
	interactive_send (ACTION_READY);
	
	while (run)
	{
		int i = 0, size = 0;
		memset (buffer, '\0', 4096);
		while ((size = fread (&byte, 1, 1, stdin)))
		{
			if (byte == '\n') break;
			buffer[i] = byte; 
			i++;
		}
		
		if (memcmp (buffer, CMD_QUIT, strlen (CMD_QUIT)) == 0 || quit || size == 0)
		{
			run = false;
			stop = true;
		}
		else if (memcmp (buffer, CMD_LAMEDB, strlen (CMD_LAMEDB)) == 0)
		{
			if (!exec)
			{
				if (strlen (buffer) > strlen (CMD_LAMEDB)+1)
				{
					strcpy (lamedb, buffer + strlen (CMD_LAMEDB)+1);
					interactive_send (ACTION_OK);
				}
				else interactive_send_text (ACTION_ERROR, "required one parameter");
			}
			else interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_EPGDAT, strlen (CMD_EPGDAT)) == 0)
		{
			if (!exec)
			{
				if (strlen (buffer) > strlen (CMD_EPGDAT)+1)
				{
					strcpy (epgdat, buffer + strlen (CMD_EPGDAT)+1);
					interactive_send (ACTION_OK);
				}
				else interactive_send_text (ACTION_ERROR, "required one parameter");
			}
			else interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_CONVERT, strlen (CMD_CONVERT)) == 0)
		{
			if (!exec)
			{
				stop = false;
				exec = true;
				pthread_create (&thread, NULL, convert, NULL);
			}
			else interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_TEXT, strlen (CMD_TEXT)) == 0)
		{
			if (!exec)
			{
				stop = false;
				exec = true;
				pthread_create (&thread, NULL, text, NULL);
			}
			else interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_STOP, strlen (CMD_STOP)) == 0)
		{
			stop = true;
			timeout_enable = true;
		}
		else
		{
			interactive_send_text (ACTION_ERROR, "unknow command");
			timeout_enable = true;
		}
		pthread_mutex_lock (&mutex);
		timeout = 0;
		pthread_mutex_unlock (&mutex);
	}
	quit = true;
	if (exec) pthread_join (thread, NULL);
	return NULL;
}

void interactive_manager ()
{
	pthread_t thread;
	quit = false;
	exec = false;
	pthread_mutex_init (&mutex, NULL);
	pthread_create (&thread, NULL, interactive, NULL);
	while (true)
	{
		pthread_mutex_lock (&mutex);
		if (exec || !timeout_enable) timeout = 0;
		else timeout++;
		if (timeout > 200)
		{
			pthread_kill (thread, SIGQUIT);
			quit = true;
		}
		pthread_mutex_unlock (&mutex);
		if (quit) break;
		usleep (100000);
	}
	pthread_join (thread, NULL);
}

int main (int argc, char **argv)
{
	int c;
	bool iactive = false;
	
	char *db_root = DEFAULT_DB_ROOT;
	
	strcpy (lamedb, DEFAULT_LAMEDB);
	strcpy (epgdat, DEFAULT_EPG_DAT);
	opterr = 0;
	
	while ((c = getopt (argc, argv, "l:e:h:d:k:r")) != -1)
	{
		switch (c)
		{
			case 'l':
				strcpy (lamedb, optarg);
				break;
			case 'e':
				strcpy (epgdat, optarg);
				break;
			case 'd':
				db_root = optarg;
				break;
			case 'k':
				nice (atoi(optarg));
				break;
			case 'r':
				log_disable ();
				interactive_enable ();
				iactive = true;
				break;
			case '?':
				printf ("Usage:\n");
				printf ("  ./crossepg_dbconverter [options]\n");
				printf ("Options:\n");
				printf ("  -l filename   lamedb filename\n");
				printf ("                default: %s\n", lamedb);
				printf ("  -e filename   epg.dat filename\n");
				printf ("                default: %s\n", epgdat);
				printf ("  -d db_root    crossepg db root folder\n");
				printf ("                default: %s\n", db_root);
				printf ("  -k nice       see \"man nice\"\n");
				printf ("  -r            interactive mode\n");
				printf ("  -h            show this help\n");
				return 0;
		}
	}
	
	mkdir (db_root, S_IRWXU|S_IRWXG|S_IRWXO);
	
	log_open (db_root);
	log_banner ("CrossEPG DB Converter");

	if (epgdb_open (db_root)) log_add ("EPGDB opened");
	else
	{
		//interactive_send_text (ACTION_ERROR, "error opening EPGDB");
		log_add ("Error opening EPGDB");
		epgdb_close ();
		return 0;
	}
	
	log_add ("Reading EPGDB...");
	if (epgdb_load ()) log_add ("Completed");
	else
	{
		//interactive_send_text (ACTION_ERROR, "cannot read EPGDB");
		log_add ("Cannot read EPGDB");
		epgdb_close ();
		return 0;
	}
	
	if (iactive) interactive_manager ();
	else write_epgdat ();
	
	epgdb_clean ();
	epgdb_close ();
	log_add ("EPGDB closed");
	memory_stats ();
	log_close ();
	
	return 0;
}
