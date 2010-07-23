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
#include "../common/epgdb/epgdb.h"
#include "../common/epgdb/epgdb_channels.h"
#include "../common/epgdb/epgdb_titles.h"

#include "dgs.h"
#include "dgs_helper.h"

plugin_info( author,    "Sandro Cavazzoni" );
plugin_info( rev,	    RELEASE );
plugin_info( desc,	    "SIFTeam CrossEPG Sync" );
plugin_info( ldesc,     "Sync CrossEPG database with the internal DGS EPG" );

#include "ui/colors.c"
#include "ui/images.c"
#include "ui/textarea.c"
#include "ui/window_box.c"
#include "ui/window_progress.c"

static time_t now;
static int progress_max;
static int count;

bool savepid ()
{
	char pid_txt[256];
	pid_t pid = getpid ();
	FILE *fd = fopen ("/tmp/crossepg.sync.pid", "w");
	if (fd == NULL) return false;
	sprintf (pid_txt, "%d", (int)pid);
	fwrite (pid_txt, strlen (pid_txt), 1, fd);
	fclose (fd);
	return true;
}

void delpid ()
{
	unlink ("/tmp/crossepg.sync.pid");
}

void add_title (epgdb_title_t *title, int id, int nid, int tsid, int sid)
{
	if (dgs_helper_adjust_daylight (title->start_time) < (now + (60*60*config_get_sync_hours ())))
	{
		bool exist = false;
		epgnode_t *epgnode = epg_repo_select_epginfo (id);
		epgnode_t *tmp = epgnode;
		while (tmp != NULL)
		{
			if (!(((dgs_helper_adjust_daylight(title->start_time) + title->length) <= tmp->epg->start_time) || (dgs_helper_adjust_daylight (title->start_time) >= (tmp->epg->start_time + tmp->epg->duration))))
			{
				exist = true;
				break;
			}

			tmp = tmp->next;
		}
	
		epg_repo_release_epgnode_list (epgnode);

		//if (!epg_repo_seek_epginfo_org_network_ts_service_id_and_start_time(nid[count], tsid[count], sid[count], title->start_time))
		if (!exist)
		{
		    epginfo_t *epginfo;
			char *name;
			char *description;
			iso639lang_t lang = {'e','n','g'};
			epginfo = malloc (sizeof (epginfo_t));
			name = epgdb_read_description (title);
			description = epgdb_read_long_description (title);

			epginfo->event_id					= title->event_id;
			epginfo->org_network_id				= nid;
			epginfo->ts_id						= tsid;
			epginfo->service_id					= sid;
			epginfo->start_time 				= dgs_helper_adjust_daylight (title->start_time);
			epginfo->duration					= title->length;
			epginfo->short_evnet_langcode[0]	= lang[0]; 
			epginfo->short_evnet_langcode[1]	= lang[1]; 
			epginfo->short_evnet_langcode[2]	= lang[2]; 
			epginfo->version_number				= 255;
			epginfo->event_name					= malloc (strlen (name) + 1);
			epginfo->extend_event_langcode[0]	= lang[0]; 
			epginfo->extend_event_langcode[1]	= lang[1]; 
			epginfo->extend_event_langcode[2]	= lang[2]; 
			epginfo->content					= 0;
			epginfo->extend_event_desc			= malloc (strlen (description) + 1);
			epginfo->short_event_desc			= NULL;
			epginfo->parental_rating			= NULL;
			epginfo->scheduled_sch_id			= 65535;
			epginfo->scheduled_mode				= 65535;
			strcpy (epginfo->event_name, name);
			strcpy (epginfo->extend_event_desc, description);
			
			_free (name);
			_free (description);
			epg_repo_insert_epginfo (epginfo);
		}
	}
}

int _get_channels_count_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
	progress_max++;
	return 0;
}

int _get_channels_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	int nid, tsid, sid, id;
	//_dgs_helper_get_channels_t *args = (_dgs_helper_get_channels_t*)p_data;
	
	id = atoi (p_fields[0]);
	nid = atoi (p_fields[1]);
	tsid = atoi (p_fields[2]);
	sid = atoi (p_fields[3]);
	
	epgdb_channel_t *channel = epgdb_channels_get_by_freq (nid, tsid, sid);
	
	if (channel != NULL)
	{
		epgdb_title_t *title = channel->title_first;

		while (title != NULL)
		{
			add_title (title, id, nid, tsid, sid);
			title = title->next;
		}
	}
	
	count++;
	window_progress_update (intl (SYNC_EPG), "", (100 * count) / progress_max);
	
	return 0;
}

int plugin_main(int argc, char *argv[])
{
	epgdb_channel_t *channel;
	int groups[64];
	int *ptr_groups;
	int i;
	int groups_count;
	//bool is_off = false;

	now = time (NULL);
	
	savepid ();
	
	intl_init ();
	
	if (!dgs_opendb ())
	{
		//show_message_box (intl (SERIOUS_ERROR), intl (CANNOT_OPEN_DGSDB), 0);
		delpid ();
		return 0;
	}
	config_read ();
	images_load ();
	colors_read ();
	
	log_open (config_get_sync_log_file (), "CrossEPG Sync Plugin");
	
	char *lang = dgs_helper_get_menu_language ();
	if (strlen (lang) > 0)
	{
		char lang_file[256];
		sprintf (lang_file, "%s/locale/%s.lng", config_get_home_directory (), lang);
		if (!intl_read (lang_file))
			log_add ("Cannot load translations");
	}
	_free (lang);
	
	if (epgdb_open (config_get_db_root ()))
	{
		log_add ("EPGDB opened");
		log_add ("Reading EPGDB...");
		if (epgdb_load ()) log_add ("Complete");
		else
		{
			log_add ("Error reading data");
			show_message_box (intl (ERROR), intl (ERROR_READ_EPGDB), 0);
			epgdb_clean ();
			epgdb_close ();
			dgs_closedb ();
			images_clean ();
			log_add ("CrossEPG Sync Plugin ended");
			log_close ();
			delpid ();
			return 0;
		}
	}
	else
	{
		log_add ("Error opening EPGDB");
		show_message_box (intl (ERROR), intl (ERROR_OPEN_EPGDB), 0);
		epgdb_close ();
		dgs_closedb ();
		images_clean ();
		log_add ("CrossEPG Sync Plugin ended");
		log_close ();
		delpid ();
		return 0;
	}
	
	//if (dgs_helper_power_status () == -1) is_off = true;

	//if (is_off) dgs_helper_power_on ();
	
	window_progress_init ();
	window_progress_update (intl (SYNC_EPG), "", 0);
	window_progress_show ();
	
	ptr_groups = config_get_sync_groups ();
	groups_count = 0;
	for (i=0; i<64; i++)
	{
		if (ptr_groups[i] > 0)
		{
			groups[groups_count] = ptr_groups[i];
			groups_count++;
		}
	}
	
	if (groups_count > 0) log_add ("Reading channels list with group filter (%d groups)", groups_count);
	else log_add ("Reading channels list without group filter");
	log_add ("Hours filter is setted to %d", config_get_sync_hours ());
	
	if (groups_count == 0)
	{
		count = 0;
		progress_max = epgdb_channels_count ();
		channel = epgdb_channels_get_first ();
		while (channel != NULL)
		{
			int i;
			int id = dgs_helper_get_channel (channel->nid, channel->tsid, channel->sid);
			if (id > 0)
			{
				epgdb_title_t *title = channel->title_first;
				
				while (title != NULL)
				{
					add_title (title, id, channel->nid, channel->tsid, channel->sid);
					title = title->next;
				}
			}
			
			for (i=0; i<channel->aliases_count; i++)
			{
				int id = dgs_helper_get_channel (channel->aliases[i].nid, channel->aliases[i].tsid, channel->aliases[i].sid);
				if (id > 0)
				{
					epgdb_title_t *title = channel->title_first;
					
					while (title != NULL)
					{
						add_title (title, id, channel->aliases[i].nid, channel->aliases[i].tsid, channel->aliases[i].sid);
						title = title->next;
					}
				}
			}
			channel = channel->next;
			count++;
			window_progress_update (intl (SYNC_EPG), "", (100 * count) / progress_max);
		}
	}
	else
	{
		if (dgs_getdb () != NULL)
		{
			int res;
		    char *errmsg = 0;
			char sql[16*1024];
			char filter[16*1024];
			char *tmp = filter;
			for (i=0; i<groups_count; i++)
			{
				if (i == (groups_count-1))
					sprintf (tmp, "FAVListInfo.grp_id = '%d'", groups[i]);
				else
					sprintf (tmp, "FAVListInfo.grp_id = '%d' OR ", groups[i]);

				tmp += strlen (tmp);
			}
			sprintf (sql,
					"SELECT	channelinfo.id, channelinfo.org_network_id, channelinfo.ts_id, channelinfo.service_id FROM channelinfo INNER JOIN FAVListInfo ON channelinfo.id = FAVListInfo.ch_id WHERE %s", filter);
			
			count = 0;
			progress_max = 0;
			res = sqlite3_exec (dgs_getdb (), sql, _get_channels_count_callback, NULL, &errmsg);
			
			if (res != SQLITE_OK)
				log_add ("SQL error: %s", errmsg);

			res = sqlite3_exec (dgs_getdb (), sql, _get_channels_callback, NULL, &errmsg);
			
			if (res != SQLITE_OK)
				log_add ("SQL error: %s", errmsg);
		}
	}

	window_progress_update (intl (COMPLETED), "", 100);
	
	sleep (2);
	
	window_progress_clean ();
	
	//if (is_off) dgs_helper_power_off ();
	
	images_clean ();
	epgdb_clean ();
	epgdb_close ();
	log_add ("EPGDB closed");
	dgs_closedb ();
	memory_stats ();
	log_add ("CrossEPG Sync ended");
	log_close ();
	
	delpid ();
	return 0;
}
