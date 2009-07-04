static unsigned int COLOR_WINDOW_TOP_RED_TAB;
static unsigned int COLOR_WINDOW_TOP_GREEN_TAB;
static unsigned int COLOR_WINDOW_TOP_YELLOW_TAB;
static unsigned int COLOR_WINDOW_TOP_BLUE_TAB;
static unsigned int COLOR_WINDOW_TOP_CLOCK;
static unsigned int COLOR_WINDOW_BOTTOM_TEXT;
static unsigned int COLOR_WINDOW_BACKGROUND;

static unsigned int COLOR_GRID_HEADERS_FOREGROUND;
static unsigned int COLOR_GRID_HEADERS_BACKGROUND;
static unsigned int COLOR_GRID_TITLES_FOREGROUND;
static unsigned int COLOR_GRID_TITLES_BACKGROUND;
static unsigned int COLOR_GRID_TITLES_SELECTED_FOREGROUND;
static unsigned int COLOR_GRID_TITLES_SELECTED_BACKGROUND;
static unsigned int COLOR_GRID_TITLES_REC_FOREGROUND;
static unsigned int COLOR_GRID_TITLES_REC_BACKGROUND;
static unsigned int COLOR_GRID_TITLES_ZAP_FOREGROUND;
static unsigned int COLOR_GRID_TITLES_ZAP_BACKGROUND;
static unsigned int COLOR_GRID_CHANNELS_FOREGROUND;
static unsigned int COLOR_GRID_CHANNELS_BACKGROUND;
static unsigned int COLOR_GRID_CHANNELS_SELECTED_FOREGROUND;
static unsigned int COLOR_GRID_CHANNELS_SELECTED_BACKGROUND;
static unsigned int COLOR_GRID_BORDER;

static unsigned int COLOR_INFO_FOREGROUND;
static unsigned int COLOR_INFO_BACKGROUND;

static unsigned int COLOR_LIST_CHANNELS_FOREGROUND;
static unsigned int COLOR_LIST_CHANNELS_BACKGROUND;
static unsigned int COLOR_LIST_CHANNELS_SELECTED_FOREGROUND;
static unsigned int COLOR_LIST_CHANNELS_SELECTED_BACKGROUND;

static unsigned int COLOR_LIST_TITLES_FOREGROUND;
static unsigned int COLOR_LIST_TITLES_BACKGROUND;
static unsigned int COLOR_LIST_TITLES_SELECTED_FOREGROUND;
static unsigned int COLOR_LIST_TITLES_SELECTED_BACKGROUND;
static unsigned int COLOR_LIST_TITLES_REC_FOREGROUND;
static unsigned int COLOR_LIST_TITLES_REC_BACKGROUND;
static unsigned int COLOR_LIST_TITLES_ZAP_FOREGROUND;
static unsigned int COLOR_LIST_TITLES_ZAP_BACKGROUND;

static unsigned int COLOR_LIST_SCHEDULER_FOREGROUND;
static unsigned int COLOR_LIST_SCHEDULER_BACKGROUND;
static unsigned int COLOR_LIST_SCHEDULER_SELECTED_FOREGROUND;
static unsigned int COLOR_LIST_SCHEDULER_SELECTED_BACKGROUND;

static unsigned int COLOR_LIGHT_BACKGROUND;
static unsigned int COLOR_LIGHT_FOREGROUND;
static unsigned int COLOR_LIGHT_REC_FOREGROUND;
static unsigned int COLOR_LIGHT_ZAP_FOREGROUND;
static unsigned int COLOR_LIGHT_CHANNEL_FOREGROUND;

static unsigned int COLOR_PROGRESS_FOREGROUND;
static unsigned int COLOR_PROGRESS_BACKGROUND;
static unsigned int COLOR_PROGRESS_BORDER;

static unsigned int COLOR_CONFIG_FOREGROUND;
static unsigned int COLOR_CONFIG_BACKGROUND;
static unsigned int COLOR_CONFIG_BORDER;
static unsigned int COLOR_CONFIG_SELECTED_FOREGROUND;
static unsigned int COLOR_CONFIG_SELECTED_BACKGROUND;

static unsigned int COLOR_MENU_FOREGROUND;
static unsigned int COLOR_MENU_BACKGROUND;
static unsigned int COLOR_MENU_BORDER;
static unsigned int COLOR_MENU_SELECTED_FOREGROUND;
static unsigned int COLOR_MENU_SELECTED_BACKGROUND;

static unsigned int COLOR_BOX_FOREGROUND;
static unsigned int COLOR_BOX_BACKGROUND;
static unsigned int COLOR_BOX_BORDER;

static unsigned int COLOR_PLUGINS_TOP_FOREGROUND;
static unsigned int COLOR_PLUGINS_TOP_BACKGROUND;
static unsigned int COLOR_PLUGINS_BACKGROUND;
static unsigned int COLOR_PLUGINS_BORDER;
static unsigned int COLOR_PLUGINS_RED;
static unsigned int COLOR_PLUGINS_GREEN;
static unsigned int COLOR_PLUGINS_YELLOW;
static unsigned int COLOR_PLUGINS_BLUE;


static char *colors_trim_spaces (char *text)
{
	char *tmp = text;
	while (tmp[0] == ' ') tmp++;
	while (strlen (tmp) > 1)
		if (tmp[strlen (tmp) - 1] == ' ') tmp[strlen (tmp) - 1] = '\0';
		else break;
	
	if (tmp[0] == ' ') tmp[0] = '\0';
	return tmp;
}

bool colors_read ()
{
	FILE *fd = NULL;
	char line[512];
	char key[256];
	unsigned int value;
	char color_file[512];
	
	/* init default colors */
	COLOR_WINDOW_TOP_RED_TAB = 0x00ff0000;
	COLOR_WINDOW_TOP_GREEN_TAB = 0x0000ff00;
	COLOR_WINDOW_TOP_YELLOW_TAB = 0x00fffc00;
	COLOR_WINDOW_TOP_BLUE_TAB = 0x000090ff;
	COLOR_WINDOW_TOP_CLOCK = 0x00ffffff;
	COLOR_WINDOW_BOTTOM_TEXT = 0x00ffffff;
	COLOR_WINDOW_BACKGROUND = 0x0003123e;
	
	COLOR_GRID_HEADERS_FOREGROUND = 0x00ffffff;
	COLOR_GRID_HEADERS_BACKGROUND = 0x00ff00ff;
	COLOR_GRID_TITLES_FOREGROUND = 0x00ffffff;
	COLOR_GRID_TITLES_BACKGROUND = 0x00ff00ff;
	COLOR_GRID_TITLES_SELECTED_FOREGROUND = 0x004f3e20;
	COLOR_GRID_TITLES_SELECTED_BACKGROUND = 0x00c0c0c0;
	COLOR_GRID_TITLES_REC_FOREGROUND = 0x00575556;
	COLOR_GRID_TITLES_REC_BACKGROUND = 0x00f38484;
	COLOR_GRID_TITLES_ZAP_FOREGROUND = 0x00575556;
	COLOR_GRID_TITLES_ZAP_BACKGROUND = 0x0095c1ea;
	COLOR_GRID_CHANNELS_FOREGROUND = 0x00ffffff;
	COLOR_GRID_CHANNELS_BACKGROUND = 0x00ff00ff;
	COLOR_GRID_CHANNELS_SELECTED_FOREGROUND = 0x004f3e20;
	COLOR_GRID_CHANNELS_SELECTED_BACKGROUND = 0x00c0c0c0;
	COLOR_GRID_BORDER = 0x009db9c8;
	
	COLOR_INFO_FOREGROUND = 0x00ffffff;
	COLOR_INFO_BACKGROUND = 0x00ff00ff;
	
	COLOR_LIST_CHANNELS_FOREGROUND = 0x00ffffff;
	COLOR_LIST_CHANNELS_BACKGROUND = 0x00ff00ff;
	COLOR_LIST_CHANNELS_SELECTED_FOREGROUND = 0x00333333;
	COLOR_LIST_CHANNELS_SELECTED_BACKGROUND = 0x00cccccc;
	
	COLOR_LIST_TITLES_FOREGROUND = 0x00ffffff;
	COLOR_LIST_TITLES_BACKGROUND = 0x00ff00ff;
	COLOR_LIST_TITLES_SELECTED_FOREGROUND = 0x00333333;
	COLOR_LIST_TITLES_SELECTED_BACKGROUND = 0x00cccccc;
	COLOR_LIST_TITLES_REC_FOREGROUND = 0x00ff0000;
	COLOR_LIST_TITLES_REC_BACKGROUND = 0x00ff00ff;
	COLOR_LIST_TITLES_ZAP_FOREGROUND = 0x007777ff;
	COLOR_LIST_TITLES_ZAP_BACKGROUND = 0x007777ff;
	
	COLOR_LIST_SCHEDULER_FOREGROUND = 0x00ffffff;
	COLOR_LIST_SCHEDULER_BACKGROUND = 0x00ff00ff;
	COLOR_LIST_SCHEDULER_SELECTED_FOREGROUND = 0x00333333;
	COLOR_LIST_SCHEDULER_SELECTED_BACKGROUND = 0x00cccccc;
	
	COLOR_LIGHT_BACKGROUND = 0x00ffffff;
	COLOR_LIGHT_FOREGROUND = 0x00ff00ff;
	COLOR_LIGHT_REC_FOREGROUND = 0x00333333;
	COLOR_LIGHT_ZAP_FOREGROUND = 0x00cccccc;
	COLOR_LIGHT_CHANNEL_FOREGROUND = 0x00fffe88;
	
	COLOR_PROGRESS_FOREGROUND = 0x00cccccc;
	COLOR_PROGRESS_BACKGROUND = 0x0003123e;
	COLOR_PROGRESS_BORDER = 0x00cccccc;
	
	COLOR_CONFIG_FOREGROUND = 0x00cccccc;
	COLOR_CONFIG_BACKGROUND = 0x0003123e;
	COLOR_CONFIG_BORDER = 0x00cccccc;
	COLOR_CONFIG_SELECTED_FOREGROUND = 0x004f3e20;
	COLOR_CONFIG_SELECTED_BACKGROUND = 0x00cccccc;
	
	COLOR_MENU_FOREGROUND = 0x00ffffff;
	COLOR_MENU_BACKGROUND = 0x0003123e;
	COLOR_MENU_BORDER = 0x00cccccc;
	COLOR_MENU_SELECTED_FOREGROUND = 0x004f3e20;
	COLOR_MENU_SELECTED_BACKGROUND = 0x00cccccc;
	
	COLOR_BOX_FOREGROUND = 0x00cccccc;
	COLOR_BOX_BACKGROUND = 0x0003123e;
	COLOR_BOX_BORDER = 0x00cccccc;
	
	COLOR_PLUGINS_TOP_FOREGROUND = 0x00cccccc;
	COLOR_PLUGINS_TOP_BACKGROUND = 0x0003123e;
	COLOR_PLUGINS_BACKGROUND = 0x0003123e;
	COLOR_PLUGINS_BORDER = 0x00cccccc;
	COLOR_PLUGINS_RED = 0x00ff0000;
	COLOR_PLUGINS_GREEN = 0x0000ff00;
	COLOR_PLUGINS_YELLOW = 0x00fffc00;
	COLOR_PLUGINS_BLUE = 0x000090ff;
	
	/* load colors from file */
	sprintf (color_file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "colors.scheme");
	
	fd = fopen (color_file, "r");
	if (!fd) 
		return false;

	while (fgets (line, sizeof(line), fd)) 
	{
		char *tmp_key;
		
		memset (key, 0, sizeof (key));
		
		if (sscanf (line, "%[^# ] %x\n", key, &value) != 2)
			if (sscanf (line, "%[^#\t]\t%x\n", key, &value) != 2)
				continue;
		
		tmp_key = colors_trim_spaces (key);
		if (strcmp (tmp_key, "window_top_red_tab") == 0) COLOR_WINDOW_TOP_RED_TAB = value;
		else if (strcmp (tmp_key, "window_top_green_tab") == 0) COLOR_WINDOW_TOP_GREEN_TAB = value;
		else if (strcmp (tmp_key, "window_top_yellow_tab") == 0) COLOR_WINDOW_TOP_YELLOW_TAB = value;
		else if (strcmp (tmp_key, "window_top_blue_tab") == 0) COLOR_WINDOW_TOP_BLUE_TAB = value;
		else if (strcmp (tmp_key, "window_top_clock") == 0) COLOR_WINDOW_TOP_CLOCK = value;
		else if (strcmp (tmp_key, "window_bottom_text") == 0) COLOR_WINDOW_BOTTOM_TEXT = value;
		else if (strcmp (tmp_key, "window_background") == 0) COLOR_WINDOW_BACKGROUND = value;
		else if (strcmp (tmp_key, "grid_header_foreground") == 0) COLOR_GRID_HEADERS_FOREGROUND = value;
		else if (strcmp (tmp_key, "grid_header_background") == 0) COLOR_GRID_HEADERS_BACKGROUND = value;
		else if (strcmp (tmp_key, "grid_title_foreground") == 0) COLOR_GRID_TITLES_FOREGROUND = value;
		else if (strcmp (tmp_key, "grid_title_background") == 0) COLOR_GRID_TITLES_BACKGROUND = value;
		else if (strcmp (tmp_key, "grid_title_selected_foreground") == 0) COLOR_GRID_TITLES_SELECTED_FOREGROUND = value;
		else if (strcmp (tmp_key, "grid_title_selected_background") == 0) COLOR_GRID_TITLES_SELECTED_BACKGROUND = value;
		else if (strcmp (tmp_key, "grid_title_rec_foreground") == 0) COLOR_GRID_TITLES_REC_FOREGROUND = value;
		else if (strcmp (tmp_key, "grid_title_rec_background") == 0) COLOR_GRID_TITLES_REC_BACKGROUND = value;
		else if (strcmp (tmp_key, "grid_title_zap_foreground") == 0) COLOR_GRID_TITLES_ZAP_FOREGROUND = value;
		else if (strcmp (tmp_key, "grid_title_zap_background") == 0) COLOR_GRID_TITLES_ZAP_BACKGROUND = value;
		else if (strcmp (tmp_key, "grid_channel_foreground") == 0) COLOR_GRID_CHANNELS_FOREGROUND = value;
		else if (strcmp (tmp_key, "grid_channel_background") == 0) COLOR_GRID_CHANNELS_BACKGROUND = value;
		else if (strcmp (tmp_key, "grid_channel_selected_foreground") == 0) COLOR_GRID_CHANNELS_SELECTED_FOREGROUND = value;
		else if (strcmp (tmp_key, "grid_channel_selected_background") == 0) COLOR_GRID_CHANNELS_SELECTED_BACKGROUND = value;
		else if (strcmp (tmp_key, "grid_border") == 0) COLOR_GRID_BORDER = value;
		else if (strcmp (tmp_key, "info_foreground") == 0) COLOR_INFO_FOREGROUND = value;
		else if (strcmp (tmp_key, "info_background") == 0) COLOR_INFO_BACKGROUND = value;
		else if (strcmp (tmp_key, "list_channels_foreground") == 0) COLOR_LIST_CHANNELS_FOREGROUND = value;
		else if (strcmp (tmp_key, "list_channels_background") == 0) COLOR_LIST_CHANNELS_BACKGROUND = value;
		else if (strcmp (tmp_key, "list_channels_selected_foreground") == 0) COLOR_LIST_CHANNELS_SELECTED_FOREGROUND = value;
		else if (strcmp (tmp_key, "list_channels_selected_background") == 0) COLOR_LIST_CHANNELS_SELECTED_BACKGROUND = value;
		else if (strcmp (tmp_key, "list_titles_foreground") == 0) COLOR_LIST_TITLES_FOREGROUND = value;
		else if (strcmp (tmp_key, "list_titles_background") == 0) COLOR_LIST_TITLES_BACKGROUND = value;
		else if (strcmp (tmp_key, "list_titles_selected_foreground") == 0) COLOR_LIST_TITLES_SELECTED_FOREGROUND = value;
		else if (strcmp (tmp_key, "list_titles_selected_background") == 0) COLOR_LIST_TITLES_SELECTED_BACKGROUND = value;
		else if (strcmp (tmp_key, "list_titles_rec_foreground") == 0) COLOR_LIST_TITLES_REC_FOREGROUND = value;
		else if (strcmp (tmp_key, "list_titles_rec_background") == 0) COLOR_LIST_TITLES_REC_BACKGROUND = value;
		else if (strcmp (tmp_key, "list_titles_zap_foreground") == 0) COLOR_LIST_TITLES_ZAP_FOREGROUND = value;
		else if (strcmp (tmp_key, "list_titles_zap_background") == 0) COLOR_LIST_TITLES_ZAP_BACKGROUND = value;
		else if (strcmp (tmp_key, "list_scheduler_foreground") == 0) COLOR_LIST_SCHEDULER_FOREGROUND = value;
		else if (strcmp (tmp_key, "list_scheduler_background") == 0) COLOR_LIST_SCHEDULER_BACKGROUND = value;
		else if (strcmp (tmp_key, "list_scheduler_selected_foreground") == 0) COLOR_LIST_SCHEDULER_SELECTED_FOREGROUND = value;
		else if (strcmp (tmp_key, "list_scheduler_selected_background") == 0) COLOR_LIST_SCHEDULER_SELECTED_BACKGROUND = value;
		else if (strcmp (tmp_key, "light_foreground") == 0) COLOR_LIGHT_FOREGROUND = value;
		else if (strcmp (tmp_key, "light_background") == 0) COLOR_LIGHT_BACKGROUND = value;
		else if (strcmp (tmp_key, "light_zap_foreground") == 0) COLOR_LIGHT_REC_FOREGROUND = value;
		else if (strcmp (tmp_key, "light_rec_foreground") == 0) COLOR_LIGHT_ZAP_FOREGROUND = value;
		else if (strcmp (tmp_key, "light_channel_foreground") == 0) COLOR_LIGHT_CHANNEL_FOREGROUND = value;
		else if (strcmp (tmp_key, "progress_foreground") == 0) COLOR_PROGRESS_FOREGROUND = value;
		else if (strcmp (tmp_key, "progress_background") == 0) COLOR_PROGRESS_BACKGROUND = value;
		else if (strcmp (tmp_key, "progress_border") == 0) COLOR_PROGRESS_BORDER = value;
		else if (strcmp (tmp_key, "config_foreground") == 0) COLOR_CONFIG_FOREGROUND = value;
		else if (strcmp (tmp_key, "config_background") == 0) COLOR_CONFIG_BACKGROUND = value;
		else if (strcmp (tmp_key, "config_border") == 0) COLOR_CONFIG_BORDER = value;
		else if (strcmp (tmp_key, "config_selected_foreground") == 0) COLOR_CONFIG_SELECTED_FOREGROUND = value;
		else if (strcmp (tmp_key, "config_selected_background") == 0) COLOR_CONFIG_SELECTED_BACKGROUND = value;
		else if (strcmp (tmp_key, "menu_foreground") == 0) COLOR_MENU_FOREGROUND = value;
		else if (strcmp (tmp_key, "menu_background") == 0) COLOR_MENU_BACKGROUND = value;
		else if (strcmp (tmp_key, "menu_border") == 0) COLOR_MENU_BORDER = value;
		else if (strcmp (tmp_key, "menu_selected_foreground") == 0) COLOR_MENU_SELECTED_FOREGROUND = value;
		else if (strcmp (tmp_key, "menu_selected_background") == 0) COLOR_MENU_SELECTED_BACKGROUND = value;
		else if (strcmp (tmp_key, "box_foreground") == 0) COLOR_BOX_FOREGROUND = value;
		else if (strcmp (tmp_key, "box_background") == 0) COLOR_BOX_BACKGROUND = value;
		else if (strcmp (tmp_key, "box_border") == 0) COLOR_BOX_BORDER = value;
		else if (strcmp (tmp_key, "plugins_top_foreground") == 0) COLOR_PLUGINS_TOP_FOREGROUND = value;
		else if (strcmp (tmp_key, "plugins_top_background") == 0) COLOR_PLUGINS_TOP_BACKGROUND = value;
		else if (strcmp (tmp_key, "plugins_background") == 0) COLOR_PLUGINS_BACKGROUND = value;
		else if (strcmp (tmp_key, "plugins_border") == 0) COLOR_PLUGINS_BORDER = value;
		else if (strcmp (tmp_key, "plugins_red") == 0) COLOR_PLUGINS_RED = value;
		else if (strcmp (tmp_key, "plugins_green") == 0) COLOR_PLUGINS_GREEN = value;
		else if (strcmp (tmp_key, "plugins_yellow") == 0) COLOR_PLUGINS_YELLOW = value;
		else if (strcmp (tmp_key, "plugins_blue") == 0) COLOR_PLUGINS_BLUE = value;
	}

	fclose (fd);
	
	return true;
}
