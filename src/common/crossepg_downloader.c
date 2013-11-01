#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#ifdef E1
#include <ost/dmx.h>
#define dmx_pes_filter_params dmxPesFilterParams
#define dmx_sct_filter_params dmxSctFilterParams
#else
#include <linux/dvb/dmx.h>
#endif

#include "../common.h"

#include "core/log.h"
#include "core/interactive.h"
#include "dvb/dvb.h"
#include "aliases/aliases.h"

#include "opentv/opentv.h"
#include "opentv/huffman.h"
#include "providers/providers.h"

#include "epgdb/epgdb.h"
#include "epgdb/epgdb_index.h"
#include "epgdb/epgdb_channels.h"
#include "epgdb/epgdb_titles.h"

#include "xmltv/xmltv_encodings.h"
#include "xmltv/xmltv_channels.h"
#include "xmltv/xmltv_parser.h"
#include "xmltv/xmltv_downloader.h"

#include "dbmerge/dbmerge.h"

buffer_t buffer[65536];
unsigned short buffer_index;
unsigned int buffer_size;
unsigned int buffer_size_last;
bool huffman_debug_titles = false;
bool huffman_debug_summaries = false;

char *db_root = DEFAULT_DB_ROOT;
char demuxer[256];
char provider[256];
char homedir[256];
int frontend = 0;

static volatile bool stop = false;
static volatile bool exec = false;
static volatile bool quit = false;
static volatile bool timeout_enable = true;
pthread_mutex_t mutex;
int timeout = 0;

bool bat_callback (int size, unsigned char* data)
{
	if (data[0] == 0x4a) opentv_read_channels_bat (data, size);
	interactive_send_int (ACTION_CHANNELS, opentv_channels_count ());
	return !stop;
}

static void format_size (char *string, int size)
{
	if (size > (1024*1024))
	{
		int sz = size / (1024*1024);
		int dc = (size % (1024*1024)) / (1024*10);
		if (dc > 0)
		{
			if (dc < 10)
				sprintf (string, "%d.0%d MB", sz, dc);
			else if (dc < 100)
				sprintf (string, "%d.%d MB", sz, dc);
			else
				sprintf (string, "%d.99 MB", sz);
		}
		else
			sprintf (string, "%d MB", sz);
	}
	else if (size > 1024)
		sprintf (string, "%d KB", (size / 1024));
	else
		sprintf (string, "%d bytes", size);
}

char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

bool opentv_titles_callback (int size, unsigned char* data)
{
	char fsize[256];
	if ((data[0] != 0xa0) && (data[0] != 0xa1) && (data[0] != 0xa2) && (data[0] != 0xa3)) return !stop;
	buffer[buffer_index].size = size;
	buffer[buffer_index].data = _malloc (size);
	memcpy(buffer[buffer_index].data, data, size);
	buffer_index++;
	buffer_size += size;
	if (buffer_size_last + 100000 < buffer_size)
	{
		format_size (fsize, buffer_size);
		interactive_send_text (ACTION_SIZE, fsize);
		buffer_size_last = buffer_size;
	}
	return !stop;
}

bool opentv_summaries_callback (int size, unsigned char* data)
{
	char fsize[256];
	buffer[buffer_index].size = size;
	buffer[buffer_index].data = _malloc (size);
	memcpy(buffer[buffer_index].data, data, size);
	buffer_index++;
	buffer_size += size;
	if (buffer_size_last + 100000 < buffer_size)
	{
		format_size (fsize, buffer_size);
		interactive_send_text (ACTION_SIZE, fsize);
		buffer_size_last = buffer_size;
	}
	return !stop;
}

void event_callback (int type, char *arg)
{
	switch (type)
	{
	case 0:
		interactive_send_text (ACTION_PROGRESS, "ON");
		break;

	case 1:
		interactive_send_text (ACTION_PROGRESS, "OFF");
		break;

	case 2:
		interactive_send_text (ACTION_TYPE, "DOWNLOADING CHANNELS LIST");
		interactive_send_text (ACTION_URL, arg);
		break;

	case 3:
		interactive_send_text (ACTION_TYPE, "DEFLATING CHANNELS LIST");
		break;

	case 4:
		interactive_send_text (ACTION_TYPE, "PARSING CHANNELS LIST");
		break;

	case 5:
		interactive_send_text (ACTION_TYPE, "DOWNLOADING EVENTS");
		interactive_send_text (ACTION_URL, arg);
		break;

	case 6:
		interactive_send_text (ACTION_TYPE, "DEFLATING EVENTS");
		break;

	case 7:
		interactive_send_text (ACTION_TYPE, "PARSING EVENTS");
		break;

	case 8:
		interactive_send_text (ACTION_TYPE, "DOWNLOADING XEPGDB");
		interactive_send_text (ACTION_URL, arg);
		break;

	case 9:
		interactive_send_text (ACTION_TYPE, "PARSING XEPGDB");
		interactive_send_text (ACTION_URL, arg);
		break;

	case 10:
		interactive_send_text (ACTION_TYPE, "DEFLATING XEPGDB");
		interactive_send_text (ACTION_URL, arg);
		break;
	}
}

void progress_callback (int value, int max)
{
	static int last = 0;
	static time_t lasttime = 0;
	int now = (value*100)/max;
	if (now != last && lasttime != time (NULL))
	{
		lasttime = time (NULL);
		interactive_send_int (ACTION_PROGRESS, now);
		last = now;
	}
}

void download_opentv ()
{
	int i;
	dvb_t settings;
	char dictionary[256];

	interactive_send (ACTION_START);
	log_add ("Started OpenTV events download");

	sprintf (dictionary, "%s/providers/%s.dict", homedir, provider);
	
	opentv_init ();
	if (huffman_read_dictionary (dictionary))
	{
		char size[256];
		
		settings.pids = providers_get_channels_pids ();
		settings.pids_count = providers_get_channels_pids_count ();
		settings.demuxer = demuxer;
		settings.frontend = frontend;
		settings.min_length = 11;
		settings.buffer_size = 16 * 1024;
		settings.filter = 0x4a;
		settings.mask = 0xff;
		
		log_add ("Reading channels...");
		interactive_send_text (ACTION_TYPE, "READ CHANNELS");
		dvb_read (&settings, *bat_callback);
		log_add ("Read %d channels", opentv_channels_count ());
		if (stop) goto opentv_stop;
		
		settings.pids = providers_get_titles_pids ();
		settings.pids_count = providers_get_titles_pids_count ();
		settings.demuxer = demuxer;
		settings.frontend = frontend;
		settings.min_length = 20;
		settings.buffer_size = 16 * 1024;
		settings.filter = 0xa0;
		settings.mask = 0xfc;
		
		buffer_index = 0;
		buffer_size = 0;
		buffer_size_last = 0;
		log_add ("Reading titles...");
		interactive_send_text (ACTION_TYPE, "READ TITLES");
		dvb_read (&settings, *opentv_titles_callback);
		format_size (size, buffer_size);
		log_add ("Read %s", size);
		interactive_send_text (ACTION_SIZE, size);
		if (stop) goto opentv_stop;
		
		log_add ("Parsing titles...");
		interactive_send_text (ACTION_TYPE, "PARSE TITLES");
		interactive_send_text (ACTION_PROGRESS, "ON");
		buffer_size = 0;
		time_t lasttime = 0;
		for (i=0; i<buffer_index; i++)
		{
			if (!stop) opentv_read_titles (buffer[i].data, buffer[i].size, huffman_debug_titles);
			buffer_size += buffer[i].size;
			_free (buffer[i].data);
			if ((i % 100) == 0)
			{
				if (lasttime != time (NULL) || (i == buffer_index-1))
				{
					lasttime = time (NULL);
					format_size (size, buffer_size);
					interactive_send_text (ACTION_SIZE, size);
					interactive_send_int (ACTION_PROGRESS, (i*100)/buffer_index);
				}
			}
		}
		format_size (size, buffer_size);
		interactive_send_text (ACTION_SIZE, size);
		log_add ("Titles parsed");
		interactive_send_text (ACTION_PROGRESS, "OFF");
		if (stop) goto opentv_stop;
		
		settings.pids = providers_get_summaries_pids ();
		settings.pids_count = providers_get_summaries_pids_count ();
		settings.demuxer = demuxer;
		settings.frontend = frontend;
		settings.min_length = 20;
		settings.buffer_size = 16 * 1024;
		settings.filter = 0xa8;
		settings.mask = 0xfc;
		
		buffer_index = 0;
		buffer_size = 0;
		buffer_size_last = 0;
		log_add ("Reading summaries...");
		interactive_send_text (ACTION_TYPE, "READ SUMMARIES");
		dvb_read (&settings, *opentv_summaries_callback);
		format_size (size, buffer_size);
		log_add ("Read %s", size);
		interactive_send_text (ACTION_SIZE, size);
		if (stop) goto opentv_stop;
		
		log_add ("Parsing summaries...");
		interactive_send_text (ACTION_TYPE, "PARSE SUMMARIES");
		interactive_send_text (ACTION_PROGRESS, "ON");
		buffer_size = 0;
		lasttime = 0;
		for (i=0; i<buffer_index; i++)
		{
			if (!stop) opentv_read_summaries (buffer[i].data, buffer[i].size, huffman_debug_summaries);
			buffer_size += buffer[i].size;
			_free (buffer[i].data);
			if ((i % 100) == 0)
			{
				if (lasttime != time (NULL) || (i == buffer_index-1))
				{
					lasttime = time (NULL);
					format_size (size, buffer_size);
					interactive_send_text (ACTION_SIZE, size);
					interactive_send_int (ACTION_PROGRESS, (i*100)/buffer_index);
				}
			}
		}
		format_size (size, buffer_size);
		interactive_send_text (ACTION_SIZE, size);
		log_add ("Summaries parsed");
		interactive_send_text (ACTION_PROGRESS, "OFF");
		//if (stop) goto opentv_stop;
		
opentv_stop:
		huffman_free_dictionary ();
	}
	
	exec = false;
	opentv_cleanup ();
	interactive_send (ACTION_END);
	log_add ("Ended OpenTV events download");
}

void *download (void *args)
{
	int i;
	char opentv_file[256];

	sprintf (opentv_file, "%s/providers/%s.conf", homedir, provider);

	if (providers_read (opentv_file))
	{
		if (providers_get_protocol () == 1)
		{
			download_opentv ();
		}
		else if (providers_get_protocol () == 2)
		{
			bool channels_loaded = false;
			interactive_send (ACTION_START);
			xmltv_channels_init ();
			for (i=0; i<10; i++)
			{
				if (strlen(providers_get_xmltv_channels (i)) == 0)
				{
					log_add ("No more url available");
					log_add ("Error downloading/parsing channels file");
					break;
				}
				log_add ("Download channels from url: %s (%d)", providers_get_xmltv_channels (i), i);
				if (xmltv_downloader_channels (providers_get_xmltv_channels (i), db_root, progress_callback, event_callback, &stop))
				{
					channels_loaded = true;
					break;
				}
			}
			if (channels_loaded)
			{
				xmltv_parser_set_iso639 (providers_get_xmltv_plang ());
				for (i=0; i<10; i++)
				{
					if (strlen(providers_get_xmltv_url (i)) == 0)
					{
						log_add ("No more url available");
						log_add ("Error downloading/parsing channels file");
						break;
					}
					log_add ("Download channels from url: %s (%d)", providers_get_xmltv_channels (i), i);
					if (xmltv_downloader_events (providers_get_xmltv_url (i), db_root, progress_callback, event_callback, &stop))
						break;
				}
			}
			exec = false;
			xmltv_channels_cleanup ();
			interactive_send (ACTION_END);
		}
		else if (providers_get_protocol () == 3)
		{
			interactive_send (ACTION_START);
			dbmerge_downloader (providers_get_xepgdb_headers_url (), providers_get_xepgdb_descriptors_url (), db_root, progress_callback, event_callback, &stop);
			exec = false;
			interactive_send (ACTION_END);
		}
		else if (providers_get_protocol () == 4)
		{
			char filename[1024], tmp[4096], *tmp2, sfrontend[4];
			FILE *fp_s; char text_s[80];
			
			interactive_send (ACTION_START);
			interactive_send_text (ACTION_TYPE, "RUNNING SCRIPT");
			interactive_send_text (ACTION_URL, providers_get_script_filename ());

			tmp2 = replace_str (providers_get_script_arguments (), "%%dbroot%%", db_root);
			strcpy (tmp, tmp2);
			tmp2 = replace_str (tmp, "%%homedir%%", homedir);
			strcpy (tmp, tmp2);
			tmp2 = replace_str (tmp, "%%demuxer%%", demuxer);
			strcpy (tmp, tmp2);
			sprintf (sfrontend, "%d", frontend);
			tmp2 = replace_str (tmp, "%%frontend%%", sfrontend);
			sprintf (filename, "LD_LIBRARY_PATH=%s %s/scripts/%s %s", homedir, homedir, providers_get_script_filename (), tmp2);

			fp_s = popen(filename, "r");
			if (fp_s == NULL) {
				interactive_send_text (ACTION_ERROR, "script returned an error");
			} else {
				/* Read the output a line at a time - output it. */
				while (fgets(text_s, sizeof(text_s), fp_s) != NULL) {
					printf ("%s", text_s); // ending '\n' is managed by script
					fflush (stdout);
				}
				pclose(fp_s);
			}
				
			exec = false;
			interactive_send (ACTION_END);
		}
		else
		{
			interactive_send_text (ACTION_ERROR, "invalid provider");
			exec = false;
		}
	}
	else
	{
		interactive_send_text (ACTION_ERROR, "cannot read provider");
		exec = false;
	}

	return NULL;
}

bool db_load ()
{
	if (epgdb_open (db_root)) log_add ("EPGDB opened (root=%s)", db_root);
	else
	{
		interactive_send_text (ACTION_ERROR, "error opening EPGDB");
		log_add ("Error opening EPGDB");
		epgdb_close ();
		return false;
	}
	epgdb_load ();

	aliases_make (homedir);
	return true;
}

void db_close ()
{
	epgdb_clean ();
	epgdb_close ();
	log_add ("EPGDB closed");
}

void *interactive (void *args)
{
	char buffer[4096], byte;
	bool run = true;
	pthread_t thread;
	
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
			log_add ("Interactive: QUIT cmd received");
			run = false;
			stop = true;
		}
		else if (memcmp (buffer, CMD_OPEN, strlen (CMD_OPEN)) == 0)
		{
			log_add ("Interactive: OPEN cmd received");
			if (!db_load ())
			{
				interactive_send_text (ACTION_ERROR, "cannot open crossepg database");
				log_add ("Interactive: ERROR action sent (cannot open crossepg database)");
			}
			else
			{
				interactive_send (ACTION_OK);
				log_add ("Interactive: OK action sent");
			}
		}
		else if (memcmp (buffer, CMD_CLOSE, strlen (CMD_CLOSE)) == 0)
		{
			log_add ("Interactive: CLOSE cmd received");
			db_close ();
			interactive_send (ACTION_OK);
			log_add ("Interactive: OK action sent");
		}
		else if (memcmp (buffer, CMD_DEMUXER, strlen (CMD_DEMUXER)) == 0)
		{
			log_add ("Interactive: DEMUXER cmd received");
			if (!exec)
			{
				if (strlen (buffer) > strlen (CMD_DEMUXER)+1)
				{
					strcpy (demuxer, buffer + strlen (CMD_DEMUXER)+1);
					interactive_send (ACTION_OK);
					log_add ("Interactive: OK action sent");
				}
				else
				{
					interactive_send_text (ACTION_ERROR, "required one parameter");
					log_add ("Interactive: ERROR action sent (required one parameter)");
				}
			}
			else
			{
				interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
				log_add ("Interactive: ERROR action sent (cannot do it... other operations in background)");
			}
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_FRONTEND, strlen (CMD_FRONTEND)) == 0)
		{
			log_add ("Interactive: FRONTEND cmd received");
			if (!exec)
			{
				if (strlen (buffer) > strlen (CMD_FRONTEND)+1)
				{
					frontend = atoi (buffer + strlen (CMD_DEMUXER)+1);
					interactive_send (ACTION_OK);
					log_add ("Interactive: OK action sent");
				}
				else
				{
					interactive_send_text (ACTION_ERROR, "required one parameter");
					log_add ("Interactive: ERROR action sent (required one parameter)");
				}
			}
			else
			{
				interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
				log_add ("Interactive: ERROR action sent (cannot do it... other operations in background)");
			}
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_DOWNLOAD, strlen (CMD_DOWNLOAD)) == 0)
		{
			log_add ("Interactive: DOWNLOAD cmd received");
			if (!exec)
			{
				if (strlen (buffer) > strlen (CMD_DOWNLOAD)+1)
				{
					strcpy (provider, buffer + strlen (CMD_DOWNLOAD)+1);
					stop = false;
					exec = true;
					pthread_create (&thread, NULL, download, NULL);
				}
				else
				{
					interactive_send_text (ACTION_ERROR, "required one parameter");
					log_add ("Interactive: ERROR action sent (required one parameter)");
				}
			}
			else
			{
				interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
				log_add ("Interactive: ERROR action sent (cannot do it... other operations in background)");
			}
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_WAIT, strlen (CMD_WAIT)) == 0)
		{
			log_add ("Interactive: WAIT cmd received");
			timeout_enable = false;
		}
		else if (memcmp (buffer, CMD_SAVE, strlen (CMD_SAVE)) == 0)
		{
			log_add ("Interactive: SAVE cmd received");
			if (!exec)
			{
				timeout_enable = false;
				interactive_send (ACTION_START);
				interactive_send_text (ACTION_PROGRESS, "ON");
				if (!epgdb_save (progress_callback))
				{
					interactive_send_text (ACTION_ERROR, "cannot save data");
					log_add ("Interactive: ERROR action sent (cannot save data)");
				}
				interactive_send (ACTION_END);
				interactive_send_text (ACTION_PROGRESS, "OFF");
			}
			else
			{
				interactive_send_text (ACTION_ERROR, "cannot do it... other operations in background");
				log_add ("Interactive: ERROR action sent (cannot do it... other operations in background)");
			}
			timeout_enable = true;
		}
		else if (memcmp (buffer, CMD_STOP, strlen (CMD_STOP)) == 0)
		{
			log_add ("Interactive: STOP cmd received");
			stop = true;
			timeout_enable = true;
		}
		else
		{
			interactive_send_text (ACTION_ERROR, "unknow command");
			log_add ("Interactive: ERROR action sent (unknow command)");
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
	int c, i;
	opterr = 0;
	bool iactive = false;
	
	strcpy (homedir, argv[0]);
	for (i = strlen (homedir)-1; i >= 0; i--)
	{
		bool ended = false;
		if (homedir[i] == '/') ended = true;
		homedir[i] = '\0';
		if (ended) break;
	}

	strcpy (demuxer, DEFAULT_DEMUXER);
	strcpy (provider, DEFAULT_OTV_PROVIDER);

	while ((c = getopt (argc, argv, "h:d:x:f:l:p:k:riyz")) != -1)
	{
		switch (c)
		{
			case 'd':
				db_root = optarg;
				break;
			case 'x':
				strcpy (demuxer, optarg);
				break;
			case 'f':
				frontend = atoi(optarg);
				break;
			case 'l':
				strcpy (homedir, optarg);
				break;
			case 'i':
				printf ("WARNING! Option -i is deprecated\n");
				break;
			case 'p':
				strcpy (provider, optarg);
				break;
			case 'k':
				nice (atoi(optarg));
				break;
			case 'r':
				log_disable ();
				interactive_enable ();
				iactive = true;
				break;
			case 'y':
				huffman_debug_summaries = true;
				break;
			case 'z':
				huffman_debug_titles = true;
				break;
			case '?':
				printf ("Usage:\n");
				printf ("  ./crossepg_downloader [options]\n");
				printf ("Options:\n");
				printf ("  -d db_root    crossepg db root folder\n");
				printf ("                default: %s\n", db_root);
				printf ("  -x demuxer    dvb demuxer\n");
				printf ("                default: %s\n", demuxer);
				printf ("  -f frontend   dvb frontend\n");
				printf ("                default: %d\n", frontend);
				printf ("  -l homedir    home directory\n");
				printf ("                default: %s\n", homedir);
				printf ("  -p provider   opentv provider\n");
				printf ("                default: %s\n", provider);
				printf ("  -k nice       see \"man nice\"\n");
				printf ("  -r            interactive mode\n");
				printf ("  -y            debug mode for huffman dictionary (summaries)\n");
				printf ("  -z            debug mode for huffman dictionary (titles)\n");
				printf ("  -h            show this help\n");
				return 0;
		}
	}
	
	while (homedir[strlen (homedir) - 1] == '/') homedir[strlen (homedir) - 1] = '\0';
	while (db_root[strlen (db_root) - 1] == '/') db_root[strlen (db_root) - 1] = '\0';
	
	mkdir (db_root, S_IRWXU|S_IRWXG|S_IRWXO);
	
	log_open (db_root);
	log_banner ("CrossEPG Downloader");

	xmltv_encodings_init ();

	if (iactive) interactive_manager ();
	else
	{
		char opentv_file[256];

		sprintf (opentv_file, "%s/providers/%s.conf", homedir, provider);
		if (providers_read (opentv_file))
		{
			if (providers_get_protocol () == 1)
			{
				log_add ("Provider %s identified as opentv", provider);
				if (!db_load ())
					goto error;
				download_opentv ();
				if (epgdb_save (NULL)) log_add ("Data saved");
				else log_add ("Error saving data");
				db_close ();
			}
			else if (providers_get_protocol () == 2)
			{
				log_add ("Provider %s identified as xmltv", provider);
				log_add ("Preferred language: %s", providers_get_xmltv_plang ());
				if (!db_load ())
					goto error;

				xmltv_channels_init ();
				for (i=0; i<10; i++)
				{
					if (strlen(providers_get_xmltv_channels (i)) == 0)
					{
						log_add ("No more url available");
						log_add ("Error downloading/parsing channels file");
						goto error;
					}
					log_add ("Download channels from url: %s (%d)", providers_get_xmltv_channels (i), i);
					if (xmltv_downloader_channels (providers_get_xmltv_channels (i), db_root, NULL, NULL, &stop))
						break;
				}

				xmltv_parser_set_iso639 (providers_get_xmltv_plang ());
				for (i=0; i<10; i++)
				{
					if (strlen(providers_get_xmltv_url (i)) == 0)
					{
						log_add ("No more url available");
						log_add ("Error downloading/parsing events file");
						goto error;
					}
					log_add ("Download events from url: %s", providers_get_xmltv_url (i));
					if (xmltv_downloader_events (providers_get_xmltv_url (i), db_root, NULL, NULL, &stop))
					{
						if (epgdb_save (NULL)) log_add ("Data saved");
						else log_add ("Error saving data");
						break;
					}
				}
				xmltv_channels_cleanup ();
				db_close ();
			}
			else if (providers_get_protocol () == 3)
			{
				log_add ("Provider %s identified as xepgdb", provider);
				log_add ("Headers url: %s", providers_get_xepgdb_headers_url ());
				log_add ("Descriptors url: %s", providers_get_xepgdb_descriptors_url ());

				if (!db_load ())
					goto error;
				if (dbmerge_downloader (providers_get_xepgdb_headers_url (), providers_get_xepgdb_descriptors_url (), db_root, NULL, NULL, &stop))
				{
					if (epgdb_save (NULL)) log_add ("Data saved");
					else log_add ("Error saving data");
				}
				else
					log_add ("Error downloading/parsing xepgdb files");
				db_close ();
			}
			else if (providers_get_protocol () == 4)
			{
				char filename[1024], tmp[1024], *tmp2;
				log_add ("Provider %s identified as script", provider);
				log_add ("Script file name: %s", providers_get_script_filename ());

				tmp2 = replace_str (providers_get_script_arguments (), "%%dbroot%%", db_root);
				strcpy (tmp, tmp2);
				tmp2 = replace_str (tmp, "%%homedir%%", homedir);
				sprintf (filename, "LD_LIBRARY_PATH=%s %s/scripts/%s %s", homedir, homedir, providers_get_script_filename (), tmp2);

				log_add ("Executing script %s ...", filename);
				system (filename);
				log_add ("Script terminated");
			}
		}
		else
			log_add ("Cannot load provider configuration (%s)", opentv_file);
	}
	
	memory_stats ();
error:
	log_close ();
	return 0;
}
