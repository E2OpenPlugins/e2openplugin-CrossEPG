#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "plugin/plugin.h"
#include "_debug/debug.h"
#include "gui/gtools.h"
#include "gui/image.h"
#include "dir_navi/dir_navigator.h"
#include "event/event.h"
#include "font/font.h"
#include "ui/wm.h"
//#include "jpeg/inf.h"
#include "ch/ch.h"
#include "st_drv_inf/mp3.h"
#include "directory.h"
#include "db/db.h"
#include "db/epg.h"
#include "dvbdrv_inf/demux.h"

#include "../common.h"

#include "../common/core/log.h"
#include "../common/core/intl.h"
#include "../common/core/config.h"
#include "../common/opentv/opentv.h"
#include "../common/opentv/huffman.h"
#include "../common/opentv/opentv_config.h"
#include "../common/aliases/aliases.h"
#include "../common/importer/importer.h"

#include "../common/epgdb/epgdb.h"
#include "../common/epgdb/epgdb_channels.h"
#include "../common/epgdb/epgdb_titles.h"

#include "dgs.h"
#include "dgs_channels.h"
#include "dgs_helper.h"

#include "scheduler/scheduler.h"

plugin_info( author,    "Sandro Cavazzoni" );
plugin_info( rev,	    RELEASE );
plugin_info( desc,	    "SIFTeam CrossEPG Downloader" );
plugin_info( ldesc,     "An EPG downloader with OpenTV support" );

#include "ui/colors.c"
#include "ui/images.c"
#include "ui/textarea.c"
#include "ui/window_box.c"
#include "ui/window_progress.c"

typedef struct buffer_s
{
	unsigned short size;
	unsigned char *data;
} buffer_t;

static int step_count = 0;
static int step_index = 0;
static volatile bool stop = false;

bool savepid ()
{
	char pid_txt[256];
	pid_t pid = getpid ();
	FILE *fd = fopen ("/tmp/crossepg.downloader.pid", "w");
	if (fd == NULL) return false;
	sprintf (pid_txt, "%d", (int)pid);
	fwrite (pid_txt, strlen (pid_txt), 1, fd);
	fclose (fd);
	return true;
}

void delpid ()
{
	unlink ("/tmp/crossepg.downloader.pid");
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
			else
				sprintf (string, "%d.%d MB", sz, dc);
		}
		else
			sprintf (string, "%d MB", sz);
	}
	else if (size > 1024)
		sprintf (string, "%d KB", (size / 1024));
	else
		sprintf (string, "%d bytes", size);
}

static void opentv_download_channels ()
{
	char msg[256];
	char step[256];
	sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
	step_index++;
	
	int i;
	for (i = 0; i < opentv_config_get_channels_pids_count (); i++)
	{
		unsigned char filter[1];
		unsigned char mask[1];
		int cycles = 0;
		int errors = 0;
		dmx_t dmx = dmx_open (0);
		filter[0] = 0x4a;
		mask[0] = 0xff;
		dmx_filter_set (dmx, opentv_config_get_channels_pids ()[i], filter, mask, 1);

		sprintf (msg, intl (READING_CHANNELS), opentv_channels_count ());
		window_progress_update (step, msg, 0);
		log_add ("Reading BAT channels on table %x...", opentv_config_get_channels_pids ()[i]);
		while ((errors < 15) && (cycles < MAX_BAT_LOOP_CYCLES) && (dmx_poll (dmx, 5000) == 1))
		{
			unsigned char buf[4*1024];	// 4K buffer size
			int size = dmx_read (dmx, buf, sizeof(buf));

			if (size > 11)
			{
				if (buf[0] == 0x4a)
				{
					if (opentv_read_channels_bat (buf, size)) errors = 0;
					else errors++;
				}
				else errors++;
			}
			else errors++;

			sprintf (msg, intl (READING_CHANNELS), opentv_channels_count ());
			window_progress_update (step, msg, 0);

			cycles++;
		}
		dmx_close (dmx);
	}
	
	log_add ("Read %d channels", opentv_channels_count ());
}

static void opentv_download_titles ()
{
	char msg[256];
	char size[256];
	int titles_size = 0;
	buffer_t buffer[65536];
	unsigned short buffer_index = 0;
	char step[256];
	sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
	step_index++;
	
	int i;
	for (i = 0; i < opentv_config_get_titles_pids_count (); i++)
	{
		unsigned char filter[1];
		unsigned char mask[1];
		char first[sizeof(buffer)];
		int first_length = 0;
		int cycles = 0;
		dmx_t dmx = dmx_open (0);
		filter[0] = 0xa0;
		mask[0] = 0xfc;
		dmx_filter_set (dmx, opentv_config_get_titles_pids ()[i], filter, mask, 1);
		
		format_size (size, titles_size);
		sprintf (msg, intl (READING_TITLES), size);
		window_progress_update (step, msg, i*(100/opentv_config_get_titles_pids_count ()));
		
		log_add ("Reading titles on table %x...", opentv_config_get_titles_pids ()[i]);
		while ((cycles < MAX_OTV_LOOP_CYCLES) && (dmx_poll (dmx, 5000) == 1))
		{
			unsigned char buf[4*1024];	// 4K buffer size
			int size = dmx_read (dmx, buf, sizeof(buf));
			
			if (size < 20) { cycles++; continue; }
			if ((buf[0] != 0xa0) && (buf[0] != 0xa1) && (buf[0] != 0xa2) && (buf[0] != 0xa3)) { cycles++; continue; }
			
			if (first_length == 0)
			{
				first_length = size;
				memcpy (first, buf, size);
			}
			else if (first_length == size)
			{
				if (memcmp (buf, first, size) == 0) break;
			}
			
			titles_size += size;
			buffer[buffer_index].size = size;
			buffer[buffer_index].data = _malloc (size);
			memcpy(buffer[buffer_index].data, buf, size);
			buffer_index++;
			
			cycles++;
		}
		dmx_close (dmx);
		
		format_size (size, titles_size);
		sprintf (msg, intl (READING_TITLES), size);
		window_progress_update (step, msg, i*(100/opentv_config_get_titles_pids_count ()));
		
		log_add ("Terminated in %d cycles", cycles);
	}
	log_add ("Read %d bytes", titles_size);
	window_progress_update (step, msg, 100);
	sleep (1);
	
	sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
	step_index++;
	window_progress_update (step, intl (PARSING_TITLES), 0);
	log_add ("Parsing titles...");
	for (i=0; i<buffer_index; i++)
	{
		opentv_read_titles (buffer[i].data, buffer[i].size, false);
		_free (buffer[i].data);
		//usleep (200);
		if ((i%100) == 0)
			window_progress_update (step, intl (PARSING_TITLES), (i*100)/buffer_index);
	}
	window_progress_update (step, intl (PARSING_TITLES), 100);
	sleep (1);
}

static void opentv_download_summaries ()
{
	char size[256];
	char msg[256];
	int summaries_size = 0;
	buffer_t buffer[65536];
	unsigned short buffer_index = 0;
	char step[256];
	sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
	step_index++;
	
	int i;
	for (i = 0; i < opentv_config_get_summaries_pids_count (); i++)
	{
		unsigned char filter[1];
		unsigned char mask[1];
		char first[sizeof(buffer)];
		int first_length = 0;
		int cycles = 0;
		dmx_t dmx = dmx_open (0);
		filter[0] = 0xa8;
		mask[0] = 0xfc;
		dmx_filter_set (dmx, opentv_config_get_summaries_pids ()[i], filter, mask, 1);
		
		format_size (size, summaries_size);
		sprintf (msg, intl (READING_SUMMARIES), size);
		window_progress_update (step, msg, i*(100/opentv_config_get_titles_pids_count ()));
		
		log_add ("Reading summaries on table %x...", opentv_config_get_summaries_pids ()[i]);
		while ((cycles < MAX_OTV_LOOP_CYCLES) && (dmx_poll (dmx, 5000) == 1))
		{
			unsigned char buf[4*1024];	// 4K buffer size
			int size = dmx_read (dmx, buf, sizeof(buf));
			
			if (size < 20) { cycles++; continue; }
			
			if (first_length == 0)
			{
				first_length = size;
				memcpy (first, buf, size);
			}
			else if (first_length == size)
			{
				if (memcmp (buf, first, size) == 0) break;
			}
			
			summaries_size += size;
			buffer[buffer_index].size = size;
			buffer[buffer_index].data = _malloc (size);
			memcpy(buffer[buffer_index].data, buf, size);
			buffer_index++;
			
			cycles++;
		}
		dmx_close (dmx);
		
		format_size (size, summaries_size);
		sprintf (msg, intl (READING_SUMMARIES), size);
		window_progress_update (step, msg, i*(100/opentv_config_get_titles_pids_count ()));
		
		log_add ("Terminated in %d cycles", cycles);
	}
	log_add ("Read %d bytes", summaries_size);
	window_progress_update (step, msg, 100);
	sleep (1);
	
	sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
	step_index++;
	window_progress_update (step, intl (PARSING_SUMMARIES), 0);
	log_add ("Parsing summaries...");
	for (i=0; i<buffer_index; i++)
	{
		opentv_read_summaries (buffer[i].data, buffer[i].size, false);
		_free (buffer[i].data);
		//usleep (2000);
		if ((i%100) == 0)
			window_progress_update (step, intl (PARSING_SUMMARIES), (i*100)/buffer_index);
	}
	window_progress_update (step, intl (PARSING_SUMMARIES), 100);
	sleep (1);
}

static void opentv_download (char *dictionary)
{
	log_add ("Started OpenTV events download");	
	opentv_init ();
	
	if (huffman_read_dictionary (dictionary))
	{
		window_progress_set_show_bar (false);
		opentv_download_channels ();
		window_progress_set_show_bar (true);
		opentv_download_titles ();
		opentv_download_summaries ();
		
		huffman_free_dictionary ();
	}
	else
	{
		log_add ("Cannot load dictionary");
		show_message_box (intl (ERROR), intl (CANNOT_LOAD_DICTIONARY), 0);
		
		step_index += 5;
	}
	
	opentv_cleanup ();
	log_add ("Ended OpenTV events download");
}

void save_progress (int value, int max)
{
	static int last = 0;
	int now = (value*100)/max;
	if (now != last)
	{
		char step[256];
		sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
		window_progress_update (step, intl (SAVING_DATA), now);
		last = now;
	}
}

static char last_msg[256];
void simple_progress (int value, int max)
{
	static int last = 0;
	int now = (value*100)/max;
	if (now != last)
	{
		char step[256];
		//char msg[256];
		sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
		//sprintf (msg, "%s", last_msg);
		window_progress_update (step, last_msg, now);
		last = now;
	}
}

void import_progress (char *file)
{
	strcpy (last_msg, file);
}

void url_progress (char *url)
{
	strcpy (last_msg, url);
}

void step_next ()
{
	step_index++;
}

int plugin_main(int argc, char *argv[])
{
	char opentv_file[256];
	int ch_id, otv_id;
	//bool is_off = false;
	char step[256];
	char dictionary[256];
	int providers_count = 0;
	int i;
	
	last_msg[0] = '\0';
	
	savepid ();
	nice (20);
	
	intl_init ();
	
	if (!dgs_opendb ())
	{
		show_message_box (intl (SERIOUS_ERROR), intl (CANNOT_OPEN_DGSDB), 0);
		delpid ();
		return 0;
	}
	config_read ();
	images_load ();
	colors_read ();
	
	if (!log_open (config_get_dwnl_log_file (), "CrossEPG Downloader Plugin"))
		show_message_box (intl (ERROR), intl (CANNOT_OPEN_LOG_FILE), 0);
	
	char *lang = dgs_helper_get_menu_language ();
	if (strlen (lang) > 0)
	{
		char lang_file[256];
		sprintf (lang_file, "%s/locale/%s.lng", config_get_home_directory (), lang);
		if (!intl_read (lang_file))
			log_add ("Cannot load translations");
	}
	_free (lang);
		
	if (epgdb_open (config_get_db_root ())) log_add ("EPGDB opened");
	else
	{
		log_add ("Error opening EPGDB");
		show_message_box (intl (ERROR), intl (ERROR_OPEN_EPGDB), 0);
		images_clean ();
		epgdb_close ();
		dgs_closedb ();
		log_add ("CrossEPG Downloader Plugin ended");
		log_close ();
		delpid ();
		return 0;
	}

	epgdb_load ();
	
	scheduler_init ();
	if (!scheduler_load (config_get_db_root ())) log_add ("Cannot load scheduler db");
	
	for (i=0; i<64; i++)
		if (strlen (config_get_otv_provider (i)) > 0)
			providers_count++;
			
	step_count = (providers_count * 5) + 2 + importer_set_steps (DEFAULT_IMPORT_ROOT, step_next);
	step_index = 1;
	
	//if (dgs_helper_power_status () == -1) is_off = true;

	//if (is_off) dgs_helper_power_on ();

	ch_id = ch_watching_id (ch_mode_live); 

	window_progress_init ();
	window_progress_update ("", "", 0);
	window_progress_show ();

	aliases_make (config_get_home_directory ());

	for (i=0; i<64; i++)
	{
		if (strlen (config_get_otv_provider (i)) > 0)
		{
			sprintf (opentv_file, "%s/providers/%s.conf", config_get_home_directory (), config_get_otv_provider (i));
			if (!opentv_config_read (opentv_file))
			{
				log_add ("Cannot load provider configuration");
				show_message_box (intl (ERROR), intl (CANNOT_LOAD_PROVIDER), 0);
				step_index += 5;
			}
			else
			{
				otv_id = dgs_helper_get_channel (opentv_config_get_nid (), opentv_config_get_tsid (), opentv_config_get_sid ());
				if ((otv_id != ch_watching_id (ch_mode_live)) && (otv_id > 0))
				{
					ch_change_fg (ch_mode_live, otv_id, NULL);
					sleep (config_get_channel_sleep ());
				}
				
				sprintf (dictionary, "%s/providers/%s.dict", config_get_home_directory (), config_get_otv_provider (i));
				
				opentv_download (dictionary);
			}
		}
	}
	if ((ch_watching_id (ch_mode_live) != ch_id) && (ch_id > 0)) ch_change_fg (ch_mode_live, ch_id, NULL);

	importer_parse_directory (DEFAULT_IMPORT_ROOT, config_get_db_root (), simple_progress, import_progress, url_progress, &stop);
	
	if (step_count > 2)
	{
		sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
		window_progress_update (step, intl (SAVING_DATA), 0);
		
		log_add ("Saving data...");
		if (epgdb_save (save_progress)) log_add ("Data saved");
		else log_add ("Error saving data");
		
		step_index++;
		window_progress_set_show_bar (false);
		log_add ("Linking events...");
		sprintf (step, "%s %d/%d", intl (STEP), step_index, step_count);
		window_progress_update (step, intl (LINKING_EVENTS), 0);
		scheduler_link ();
		sleep (1);
	}
	
	window_progress_set_show_bar (false);
	window_progress_update (step, intl (COMPLETED), 100);
	sleep (2);
	window_progress_clean ();
	
	//if (is_off) dgs_helper_power_off ();

	log_add ("CrossEPG Downloader ended");

	if (!scheduler_save (config_get_db_root ())) log_add ("Cannot save scheduler db");
	scheduler_clean ();
	images_clean ();
	epgdb_clean ();
	epgdb_close ();
	memory_stats ();
	log_close ();
	dgs_closedb ();
	
	delpid ();
	return 0;
}
