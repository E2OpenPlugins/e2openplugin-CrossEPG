#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "plugin/api.h"
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

#include "../common/core/config.h"
#include "../common/core/log.h"
#include "../common/core/intl.h"
#include "../common/epgdb/epgdb.h"
#include "../common/epgdb/epgdb_channels.h"
#include "../common/epgdb/epgdb_titles.h"
#include "../common/epgdb/epgdb_search.h"
#include "dgs.h"
#include "dgs_channels.h"
#include "dgs_groups.h"
#include "dgs_scheduler.h"
#include "dgs_helper.h"
#include "scheduler/scheduler.h"

plugin_info( author,    "Sandro Cavazzoni" );
plugin_info( rev,	    RELEASE );
plugin_info( desc,	    "CrossEPG" );
plugin_info( ldesc,     "A graphical EPG" );

static dgs_channel_t *selected_channel;
static dgs_channel_t *start_channel;
static dgs_group_t *selected_group;
static epgdb_title_t *selected_title;
static epgdb_title_t *start_title;
static scheduler_t *selected_scheduler;
static scheduler_t *start_scheduler;
static int selected_screen;
static int selected_column;
// static bool selected_extra_info;

static time_t start_time;
static time_t selected_time;

#include "ui/colors.c"
#include "ui/images.c"
#include "ui/textarea.c"
#include "ui/background.c"
#include "ui/window_menu.c"
#include "ui/window_config.c"
#include "ui/window_box.c"
#include "ui/window_top.c"
#include "ui/window_bottom.c"
#include "ui/window_progress.c"
#include "ui/grid.c"
#include "ui/info.c"
#include "ui/list_channels.c"
#include "ui/list_titles.c"
#include "ui/list_scheduler.c"
#include "ui/light.c"
#include "actions.c"
#include "remotes/remote_red.c"
#include "remotes/remote_green.c"
#include "remotes/remote_yellow.c"
#include "remotes/remote_blue.c"

bool savepid ()
{
	char pid_txt[256];
	pid_t pid = getpid ();
	FILE *fd = fopen ("/tmp/crossepg.pid", "w");
	if (fd == NULL) return false;
	sprintf (pid_txt, "%d", (int)pid);
	fwrite (pid_txt, strlen (pid_txt), 1, fd);
	fclose (fd);
	return true;
}

void delpid ()
{
	unlink ("/tmp/crossepg.pid");
}

void scheduler_sync ()
{
	bool changed = false;
	dgs_scheduler_init ();
	scheduler_init ();
	
	if (!dgs_scheduler_read ())
	{
		dgs_scheduler_clean ();
		return;
	}
	if (!scheduler_load (config_get_db_root ()))
	{
		scheduler_clean ();
		dgs_scheduler_clean ();
		return;
	}
	
	scheduler_t *tmp2;
	dgs_scheduler_t *tmp = dgs_scheduler_get_first ();
	
	while (tmp != NULL)
	{
		bool exist = false;
		tmp2 = scheduler_get_first ();
		while (tmp2 != NULL)
		{
			if ((tmp2->start_time == tmp->start_time) && (tmp2->length == tmp->length) && ((tmp2->type == 0) || (tmp2->type == 1)))
			{
				exist = true;
				break;
			}
			tmp2 = tmp2->next;
		}
		if (!exist)
		{
			scheduler_add (tmp->ch_id, tmp->start_time, tmp->length, tmp->mode, tmp->name, false);
			changed = true;
		}
		tmp = tmp->next;
	}
	
	tmp2 = scheduler_get_first ();
	while (tmp2 != NULL)
	{
		if ((tmp2->type == 0) || (tmp2->type == 1))
		{
			bool exist = false;
			tmp = dgs_scheduler_get_first ();
			while (tmp != NULL)
			{
				if ((tmp2->start_time == tmp->start_time) && (tmp2->length == tmp->length))
				{
					exist = true;
					break;
				}
				tmp = tmp->next;
			}
			if (!exist)
			{
				scheduler_del (tmp2->channel_id, tmp2->start_time, tmp2->length, tmp2->type, false);
				changed = true;
			}
		}
		tmp2 = tmp2->next;
	}
	
	if (changed)
	{
		if (scheduler_save (config_get_db_root ())) log_add ("Saved scheduler list");
		else log_add ("Error saving scheduler list");
	}
	scheduler_clean ();
	dgs_scheduler_clean ();
}

int ret = 0;

void show_red ()
{
	int rep_count = 0;
	event_t event;
	
	grid_init ();
	grid_update ();
	grid_show ();
	
	/* wait for remote key press */
	plug_event_init ();
	while (true)
	{
		if (plug_event_rcv (&event) < 0)
			continue;
		
		if (event.type != event_type_key)
			continue;
		
		//plug_event_close ();
		if (event_key_2value (&event) != event_key_value_rep) rep_count = 0;
		if (rep_count > 2) rep_count = 0;
		
		if (rep_count == 0)
		{
			if ((ret = remote_red_event_handler (&event)) < 0)
				break;
			
			if (event_key_2value (&event) != event_key_value_rep)
			{
				plug_event_close ();
				grid_update ();
				info_update ();
				window_top_update ();
				plug_event_init ();
			}
			else
				grid_update ();
		}
		
		if (event_key_2value (&event) == event_key_value_rep) rep_count++;
		
		//plug_event_init ();
	}
	plug_event_close ();
	
	grid_clean ();
}

void show_green ()
{
	int rep_count = 0;
	event_t event;
	
	list_channels_init ();
	list_channels_update ();
	list_channels_show ();
	
	list_titles_init ();
	list_titles_update ();
	list_titles_show ();
	
	/* wait for remote key press */
	plug_event_init ();
	while (true)
	{
		if (plug_event_rcv (&event) < 0)
			continue;
		
		if (event.type != event_type_key)
			continue;
		
		//plug_event_close ();
		if (event_key_2value (&event) != event_key_value_rep) rep_count = 0;
		if (rep_count > 1) rep_count = 0;
		
		if (rep_count == 0)
		{
			if ((ret = remote_green_event_handler (&event)) < 0)
				break;
			
			if ((event_key_2value (&event) != event_key_value_rep) || (ret == 0))
			{
				plug_event_close ();
				list_channels_update ();
				list_titles_update ();
				info_update ();
				window_top_update ();
				plug_event_init ();
			}
			else
			{
				if (ret == 1) list_channels_update ();
				else list_titles_update ();
			}
		}
		
		if (event_key_2value (&event) == event_key_value_rep) rep_count++;
		
		//plug_event_init ();
	}
	plug_event_close ();
	
	list_titles_clean ();
	list_channels_clean ();
}

void show_yellow ()
{
	event_t event;
	
	light_show ();
	
	/* wait for remote key press */
	plug_event_init ();
	while (true)
	{
		if (plug_event_rcv (&event) < 0)
			continue;

		if (event.type != event_type_key)
			continue;

		plug_event_close ();
		if ((ret = remote_yellow_event_handler (&event)) < 0)
			break;
			
		light_update ();
		window_top_update ();
		
		plug_event_init ();
	}
	light_hide ();
}

void show_blue ()
{
	event_t event;
	
	//light_show ();
	list_scheduler_init ();
	list_scheduler_update ();
	list_scheduler_show ();
	
	/* wait for remote key press */
	plug_event_init ();
	while (true)
	{
		if (plug_event_rcv (&event) < 0)
			continue;

		if (event.type != event_type_key)
			continue;

		plug_event_close ();
		if ((ret = remote_blue_event_handler (&event)) < 0)
			break;
			
		//light_update ();
		window_top_update ();
		list_scheduler_update ();
		
		plug_event_init ();
	}
	
	list_scheduler_clean ();
	//light_hide ();
}

int plugin_main(int argc, char *argv[])
{
	int current_chid;
	int current_grpid;
	time_t tmp_time = time (NULL);
	
	savepid ();
	
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
	
	if (!log_open (config_get_log_file (), "CrossEPG Plugin"))
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
	
	scheduler_sync ();
	
	dgs_groups_init ();
	dgs_channels_init ();
	
	log_add ("Reading fav groups...");
	dgs_groups_read ();
	if (dgs_groups_count () == 0)
	{
		log_add ("No fav groups founded. The application cannot work without fav groups");
		show_message_box (intl (ERROR), intl (NO_FAV_GROUPS), 0);
		dgs_channels_clean ();
		dgs_groups_clean ();
		images_clean ();
		log_add ("CrossEPG Plugin ended");
		log_close ();
		dgs_closedb ();
		delpid ();
		return 0;
	}
	log_add ("Read %d fav groups", dgs_groups_count ());
	
	current_chid = ch_watching_id (ch_mode_live);
	current_grpid = dgs_helper_get_current_group (current_chid);
	selected_group = dgs_groups_get_by_id (current_grpid);
	current_grpid = selected_group->id;
	
	log_add ("Reading channels on fav group %d...", current_grpid);
	dgs_channels_readgroup (current_grpid);
	log_add ("Read %d channels", dgs_channels_count ());
	
	//dgs_scheduler_init ();
	//dgs_scheduler_read ();
	scheduler_init ();
	if (!scheduler_load (config_get_db_root ())) log_add ("Cannot load scheduler db");
	
	selected_channel = dgs_channels_get_by_id (current_chid);
	start_channel = selected_channel;
	selected_time = tmp_time - (tmp_time % (60*30)) + (60*30);
	start_time = tmp_time - (tmp_time % (60*30));
	selected_column = 0;
	selected_screen = config_get_start_screen ();
	
	if (epgdb_open (config_get_db_root ()))
	{
		log_add ("EPGDB opened");
		log_add ("Reading EPGDB...");
		if (epgdb_load ()) log_add ("Complete");
		else log_add ("Error reading data");
	}
	else
	{
		log_add ("Error opening EPGDB");
		show_message_box (intl (ERROR), intl (ERROR_OPEN_EPGDB), 0);
		show_config ();
		epgdb_close ();
		scheduler_clean ();
		//dgs_scheduler_clean ();
		dgs_channels_clean ();
		dgs_groups_clean ();
		dgs_closedb ();
		images_clean ();
		log_add ("CrossEPG Plugin ended");
		log_close ();
		delpid ();
		return 0;
	}
	
	dgs_channels_link_to_db ();
	if (selected_channel != NULL)
	{
		selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, tmp_time - (dgs_helper_get_daylight_saving () * (60 * 60)));
		start_title = selected_title;
	}
	else
	{
		selected_title = NULL;
		start_title = NULL;
	}
	
	while (selected_screen > -1)
	{
		switch (selected_screen)
		{
			case 0:
				dgs_helper_live_boxed ();
				show_background ("red_screen.png", 0);
				window_top_show ();
				window_bottom_show (0);
				info_show ();
				show_red ();
				break;
			case 1:
				dgs_helper_live_boxed ();
				show_background ("green_screen.png", 0);
				window_top_show ();
				window_bottom_show (0);
				info_show ();
				show_green ();
				break;
			case 2:
				info_hide ();
				dgs_helper_live_restore ();
				show_background ("yellow_screen.png", 1);
				window_top_show ();
				window_bottom_hide ();
				show_yellow ();
				break;
			case 3:
				selected_scheduler = scheduler_get_first ();
				start_scheduler = scheduler_get_first ();
				info_hide ();
				dgs_helper_live_boxed ();
				show_background ("blue_screen.png", 0);
				window_top_show ();
				window_bottom_show (1);
				show_blue ();
				break;
		}
	}
	info_hide ();
	window_bottom_hide ();
	window_top_hide ();
	hide_background ();
	dgs_helper_live_restore ();
	images_clean ();
	epgdb_clean ();
	epgdb_close ();
	
	if (!scheduler_save (config_get_db_root ())) log_add ("Cannot save scheduler db");
	scheduler_clean ();
	
	//dgs_scheduler_clean ();
	dgs_channels_clean ();
	dgs_groups_clean ();
	dgs_closedb ();
	memory_stats ();
	log_add ("CrossEPG Plugin ended");
	log_close ();
	
	if (ret == -2)
		plugin_execute (config_get_sync_file (), NULL);
	else if (ret == -3)
		plugin_execute (config_get_dwnl_file (), NULL);
	
	//sleep (2);
	delpid ();
	return 0;
}
