#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "plugin/plugin.h"
#include "plugin/api.h"
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

#include "dgs.h"
#include "dgs_helper.h"

plugin_info( author,    "Sandro Cavazzoni" );
plugin_info( rev,	    RELEASE );
plugin_info( desc,	    "CrossEPG Launcher" );
plugin_info( ldesc,     "" );

#include "ui/colors.c"
#include "ui/images.c"
#include "ui/textarea.c"
#include "ui/window_box.c"
#include "ui/window_config.c"
#include "ui/window_plugins.c"

static w_plugin_item_t plugins[256];
static unsigned char plugins_count;

static int red_plugin_index = -1;
static int green_plugin_index = -1;
static int yellow_plugin_index = -1;
static int blue_plugin_index = -1;
static int red_long_plugin_index = -1;
static int green_long_plugin_index = -1;
static int yellow_long_plugin_index = -1;
static int blue_long_plugin_index = -1;

bool launcher_extension_check (char *filename, char *extension)
{
	if (strlen (filename) <= strlen (extension) + 1) return false;
	if (memcmp (filename + (strlen (filename) - strlen (extension)), extension, strlen (extension)) != 0) return false;
	if (filename[strlen (filename) - strlen (extension) - 1] != '.') return false;
	return true;
}

void read_plugins ()
{
	DIR *dp;
	struct dirent *ep;
	
	log_add ("Loading plugins list...");
	
	dp = opendir (dir_plugin);
	if (dp != NULL)
	{
		while ((ep = readdir (dp)) != NULL)
		{
			if (launcher_extension_check (ep->d_name, "plugin"))
			{
				plugin_info_t *info = malloc (sizeof (plugin_info_t));
				char file[256];
				sprintf (file, "%s/%s", dir_plugin, ep->d_name);
				info->author = NULL;
				info->rev = NULL;
				info->desc = NULL;
				info->ldesc = NULL;
				
				//memcpy (plugins[plugins_count], ep->d_name, strlen (ep->d_name)-7);
				
				plugin_info_get(file, info);
				if (info->desc != NULL)
				{
					if (strlen (info->desc) > 5)
					{
						strcpy (plugins[plugins_count].filename, file);
						memcpy (plugins[plugins_count].name, ep->d_name, strlen (ep->d_name)-7);
						strcpy (plugins[plugins_count].desc, info->desc+5);
						plugins_count++;
					}
				}
				plugin_info_release (info);
			}
		}
		closedir (dp);
	}
	
	log_add ("Loaded %d plugins", plugins_count);
}

void sort_plugins ()
{
	int i, j;
	for (i=(plugins_count-1); i>0; i--)
	{
		for (j=0; j<i; j++)
		{
			if (strcmp (plugins[j].name, plugins[j+1].name) > 0)
			{
				char tmp[256];
				strcpy (tmp, plugins[j].name);
				strcpy (plugins[j].name, plugins[j+1].name);
				strcpy (plugins[j+1].name, tmp);
				strcpy (tmp, plugins[j].desc);
				strcpy (plugins[j].desc, plugins[j+1].desc);
				strcpy (plugins[j+1].desc, tmp);
				strcpy (tmp, plugins[j].filename);
				strcpy (plugins[j].filename, plugins[j+1].filename);
				strcpy (plugins[j+1].filename, tmp);
			}
		}
	}
}

void set_red_plugin (char *filename)
{
	int i;
	for (i=0; i<plugins_count; i++)
	{
		if (strcmp (filename, plugins[i].filename) == 0)
		{
			red_plugin_index = i;
			break;
		}
	}
}

void set_green_plugin (char *filename)
{
	int i;
	for (i=0; i<plugins_count; i++)
	{
		if (strcmp (filename, plugins[i].filename) == 0)
		{
			green_plugin_index = i;
			break;
		}
	}
}

void set_yellow_plugin (char *filename)
{
	int i;
	for (i=0; i<plugins_count; i++)
	{
		if (strcmp (filename, plugins[i].filename) == 0)
		{
			yellow_plugin_index = i;
			break;
		}
	}
}

void set_blue_plugin (char *filename)
{
	int i;
	for (i=0; i<plugins_count; i++)
	{
		if (strcmp (filename, plugins[i].filename) == 0)
		{
			blue_plugin_index = i;
			break;
		}
	}
}

void set_red_long_plugin (char *filename)
{
	int i;
	for (i=0; i<plugins_count; i++)
	{
		if (strcmp (filename, plugins[i].filename) == 0)
		{
			red_long_plugin_index = i;
			break;
		}
	}
}

void set_green_long_plugin (char *filename)
{
	int i;
	for (i=0; i<plugins_count; i++)
	{
		if (strcmp (filename, plugins[i].filename) == 0)
		{
			green_long_plugin_index = i;
			break;
		}
	}
}

void set_yellow_long_plugin (char *filename)
{
	int i;
	for (i=0; i<plugins_count; i++)
	{
		if (strcmp (filename, plugins[i].filename) == 0)
		{
			yellow_long_plugin_index = i;
			break;
		}
	}
}

void set_blue_long_plugin (char *filename)
{
	int i;
	for (i=0; i<plugins_count; i++)
	{
		if (strcmp (filename, plugins[i].filename) == 0)
		{
			blue_long_plugin_index = i;
			break;
		}
	}
}

int plugin_main(int argc, char *argv[])
{
	int i, ret = -2;
	w_plugins_t plugins_window;
	intl_init ();
	
	if (!dgs_opendb ())
	{
		show_message_box (intl (SERIOUS_ERROR), intl (CANNOT_OPEN_DGSDB), 0);
		return 0;
	}
	config_read ();
	images_load ();
	colors_read ();
	
	log_open (config_get_launcher_log_file (), "CrossEPG Launcher Plugin");
	
	char *lang = dgs_helper_get_menu_language ();
	if (strlen (lang) > 0)
	{
		char lang_file[256];
		sprintf (lang_file, "%s/locale/%s.lng", config_get_home_directory (), lang);
		if (!intl_read (lang_file))
			log_add ("Cannot load translations");
	}
	_free (lang);
	
	for (i=0; i<256; i++)
	{
		memset (plugins[i].name, 0, 256);
		memset (plugins[i].desc, 0, 256);
		memset (plugins[i].filename, 0, 256);
	}
	
	plugins_count = 0;
	
	read_plugins ();
	
	if (plugins_count > 0)
	{
		sort_plugins ();
		
		set_red_plugin (config_get_launcher_red ());
		set_green_plugin (config_get_launcher_green ());
		set_yellow_plugin (config_get_launcher_yellow ());
		set_blue_plugin (config_get_launcher_blue ());
		set_red_long_plugin (config_get_launcher_long_red ());
		set_green_long_plugin (config_get_launcher_long_green ());
		set_yellow_long_plugin (config_get_launcher_long_yellow ());
		set_blue_long_plugin (config_get_launcher_long_blue ());
		
		plugins_window.title = "CrossEPG Launcher";
		plugins_window.plugins = plugins;
		plugins_window.plugins_count = plugins_count;
		plugins_window.selected_item = 0;
		plugins_window.red_item = red_plugin_index;
		plugins_window.green_item = green_plugin_index;
		plugins_window.yellow_item = yellow_plugin_index;
		plugins_window.blue_item = blue_plugin_index;
		plugins_window.red_long_item = red_long_plugin_index;
		plugins_window.green_long_item = green_long_plugin_index;
		plugins_window.yellow_long_item = yellow_long_plugin_index;
		plugins_window.blue_long_item = blue_long_plugin_index;

		window_plugins_init (&plugins_window);
		window_plugins_update (&plugins_window);
		window_plugins_show (&plugins_window);
		ret = window_plugins_execute (&plugins_window);
		window_plugins_clean (&plugins_window);
	}
	
	images_clean ();
	dgs_closedb ();
	memory_stats ();
	log_add ("CrossEPG Launcher ended");
	log_close ();
	
	if (ret >= 0)
		plugin_execute (plugins[ret].filename, NULL);
	
	return 0;
}
