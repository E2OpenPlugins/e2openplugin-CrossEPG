#define WINDOWS_PLUGINS_WIDTH 720
#define WINDOWS_PLUGINS_HEIGHT 85
#define WINDOWS_PLUGINS_FOOTER_WIDTH 720
#define WINDOWS_PLUGINS_FOOTER_HEIGHT 60
static img_t plugin_background = NULL;

typedef struct w_plugin_item_s
{
	char name[256];
	char desc[256];
	char filename[256];
} w_plugin_item_t;

typedef struct w_plugins_s
{
	char			*title;
	w_plugin_item_t	*plugins;
	int				plugins_count;
	int				selected_item;
	int				red_item;
	int				green_item;
	int				yellow_item;
	int				blue_item;
	int				red_long_item;
	int				green_long_item;
	int				yellow_long_item;
	int				blue_long_item;
	bool			exit_ok;
	window_t		*_wnd;
	window_t		*_wnd_footer;
	window_t		*_wnd_background;
} w_plugins_t;

static void window_plugins_init (w_plugins_t *window)
{
	char file[256];
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "backgrounds/plugin.bmp");
	if (plugin_background) img_close (plugin_background);
	plugin_background = img_open_absolute_path (file);
	if (plugin_background)
	{
		rect_t rect_background = { 0, 0, 720, 576 };
		window->_wnd_background = wm_create (&rect_background, window->title, NULL, NULL, COLOR_TRANSPARENT);
		gc_set_fc (window->_wnd_background->gc, COLOR_TRANSPARENT);
		gt_fillrect (&window->_wnd_background->fb, window->_wnd_background->gc, 0, 0, 720, 576);
		img_draw (&window->_wnd_background->fb, plugin_background, window->_wnd_background->gc, 0, 0, 720, 576, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
		wm_redraw (window->_wnd_background);
	}
	rect_t rect = { 0, 0, WINDOWS_PLUGINS_WIDTH, WINDOWS_PLUGINS_HEIGHT };
	rect_t rect_footer = { 0, 576 - WINDOWS_PLUGINS_FOOTER_HEIGHT, WINDOWS_PLUGINS_FOOTER_WIDTH, WINDOWS_PLUGINS_FOOTER_HEIGHT };
	window->_wnd = wm_create (&rect, window->title, NULL, NULL, COLOR_TRANSPARENT);
	window->_wnd_footer = wm_create (&rect_footer, window->title, NULL, NULL, COLOR_TRANSPARENT);
}

static void window_plugins_show (w_plugins_t *window)
{
	if (plugin_background) wm_show (window->_wnd_background);
	wm_show (window->_wnd);	
	wm_show (window->_wnd_footer);	
}

static void window_plugins_clean (w_plugins_t *window)
{
	wm_destroy (window->_wnd);
	wm_destroy (window->_wnd_footer);
	if (plugin_background)
	{
		wm_destroy (window->_wnd_background);
		img_close (plugin_background);
		plugin_background = NULL;
	}
}

static char *ui_resize_string (char *text, int font_size, int width)
{
	int size;
	int ipotetic_font_size;
	int ipotetic_chars;
	struct _font font;
	bool dirback = false;
	char *ret;

	font.size = font_size;
	
	ipotetic_font_size = (font_size * 60) / 100;
	ipotetic_chars = width / ipotetic_font_size;
	if (ipotetic_chars > strlen (text)) ipotetic_chars = strlen (text);
	if (ipotetic_chars < 1) ipotetic_chars = 0;
	
	while (true)
	{
		size = font_width_str (&font, text, ipotetic_chars);

		if (size < width)
		{
			int forward;
			
			if (dirback) break;
			if (ipotetic_chars == strlen (text)) break;
			if ((size + ipotetic_font_size) > width) break;
			
			forward = (width - size) / ipotetic_font_size;
			if (forward <= 0) forward = 1;
			ipotetic_chars += forward;
			if (ipotetic_chars > strlen (text)) ipotetic_chars = strlen (text);
		}
		else
		{
			int back = (size - width) / ipotetic_font_size;
			if (back == 0) back = 1;
			ipotetic_chars -= back;
			if (ipotetic_chars < 1)
			{
				ipotetic_chars = 0;
				break;
			}
			dirback = true;
		}
	}

	if (ipotetic_chars == strlen (text))
	{
		ret = _malloc (ipotetic_chars + 1);
		memcpy (ret, text, ipotetic_chars);
		ret[ipotetic_chars] = '\0';
	}
	else if (ipotetic_chars < 2)
	{
		ret = _malloc (1);
		ret[0] = '\0';
	}
	else
	{
		ret = _malloc (ipotetic_chars + 2);
		memcpy (ret, text, ipotetic_chars - 2);
		ret[ipotetic_chars-2] = '.';
		ret[ipotetic_chars-1] = '.';
		ret[ipotetic_chars] = '.';
		ret[ipotetic_chars+1] = '\0';
	}
	
	return ret;
}

static void window_plugins_update (w_plugins_t *window)
{
	int offset;
	struct _font font;
	
	if (plugin_background)
	{
		gc_set_fc (window->_wnd->gc, COLOR_TRANSPARENT);
		gt_fillrect (&window->_wnd->fb, window->_wnd->gc, 0, 0, WINDOWS_PLUGINS_WIDTH, WINDOWS_PLUGINS_HEIGHT);
		
		gc_set_fc (window->_wnd_footer->gc, COLOR_TRANSPARENT);
		gt_fillrect (&window->_wnd_footer->fb, window->_wnd_footer->gc, 0, 0, WINDOWS_PLUGINS_FOOTER_WIDTH, WINDOWS_PLUGINS_FOOTER_HEIGHT);
	}
	else
	{
		gc_set_fc (window->_wnd->gc, COLOR_PLUGINS_TOP_BACKGROUND);
		gt_fillrect (&window->_wnd->fb, window->_wnd->gc, 0, 0, WINDOWS_PLUGINS_WIDTH, WINDOWS_PLUGINS_HEIGHT);

		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_BACKGROUND);
		gt_fillrect (&window->_wnd_footer->fb, window->_wnd_footer->gc, 0, 0, WINDOWS_PLUGINS_FOOTER_WIDTH, WINDOWS_PLUGINS_FOOTER_HEIGHT);
	}
	
	font.size = 12;
	
	gc_set_fc (window->_wnd->gc, COLOR_PLUGINS_TOP_FOREGROUND);
	gc_set_bc (window->_wnd->gc, COLOR_PLUGINS_TOP_BACKGROUND);
	gc_set_bc (window->_wnd_footer->gc, COLOR_PLUGINS_BACKGROUND);
	
	if (window->red_item > -1)
	{
		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_RED);
		char *text = ui_resize_string (window->plugins[window->red_item].desc, font.size, 165);
		int offset = 104 - (font_width_str (&font, text, strlen (text)) / 2);
		font_draw_str (&font, text, strlen (text), &window->_wnd_footer->fb, window->_wnd_footer->gc, offset, 10);
		_free (text);
	}

	if (window->red_long_item > -1)
	{
		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_RED);
		char *text = ui_resize_string (window->plugins[window->red_long_item].desc, font.size, 165);
		int offset = 104 - (font_width_str (&font, text, strlen (text)) / 2);
		font_draw_str (&font, text, strlen (text), &window->_wnd_footer->fb, window->_wnd_footer->gc, offset, 25);
		_free (text);
	}
	
	if (window->green_item > -1)
	{
		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_GREEN);
		char *text = ui_resize_string (window->plugins[window->green_item].desc, font.size, 165);
		int offset = 272 - (font_width_str (&font, text, strlen (text)) / 2);
		font_draw_str (&font, text, strlen (text), &window->_wnd_footer->fb, window->_wnd_footer->gc, offset, 10);
		_free (text);
	}

	if (window->green_long_item > -1)
	{
		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_GREEN);
		char *text = ui_resize_string (window->plugins[window->green_long_item].desc, font.size, 165);
		int offset = 272 - (font_width_str (&font, text, strlen (text)) / 2);
		font_draw_str (&font, text, strlen (text), &window->_wnd_footer->fb, window->_wnd_footer->gc, offset, 25);
		_free (text);
	}
	
	if (window->yellow_item > -1)
	{
		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_YELLOW);
		char *text = ui_resize_string (window->plugins[window->yellow_item].desc, font.size, 165);
		int offset = 440 - (font_width_str (&font, text, strlen (text)) / 2);
		font_draw_str (&font, text, strlen (text), &window->_wnd_footer->fb, window->_wnd_footer->gc, offset, 10);
		_free (text);
	}

	if (window->yellow_long_item > -1)
	{
		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_YELLOW);
		char *text = ui_resize_string (window->plugins[window->yellow_long_item].desc, font.size, 165);
		int offset = 440 - (font_width_str (&font, text, strlen (text)) / 2);
		font_draw_str (&font, text, strlen (text), &window->_wnd_footer->fb, window->_wnd_footer->gc, offset, 25);
		_free (text);
	}
	
	if (window->blue_item > -1)
	{
		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_BLUE);
		char *text = ui_resize_string (window->plugins[window->blue_item].desc, font.size, 165);
		int offset = 608 - (font_width_str (&font, text, strlen (text)) / 2);
		font_draw_str (&font, text, strlen (text), &window->_wnd_footer->fb, window->_wnd_footer->gc, offset, 10);
		_free (text);
	}

	if (window->blue_long_item > -1)
	{
		gc_set_fc (window->_wnd_footer->gc, COLOR_PLUGINS_BLUE);
		char *text = ui_resize_string (window->plugins[window->blue_long_item].desc, font.size, 165);
		int offset = 608 - (font_width_str (&font, text, strlen (text)) / 2);
		font_draw_str (&font, text, strlen (text), &window->_wnd_footer->fb, window->_wnd_footer->gc, offset, 25);
		_free (text);
	}
	
	gc_set_lt (window->_wnd->gc, 2);
	gt_line (&window->_wnd->fb, window->_wnd->gc, 35,  32, 25, 51);
	gt_line (&window->_wnd->fb, window->_wnd->gc, 25,  52, 35, 72);
	gt_line (&window->_wnd->fb, window->_wnd->gc, WINDOWS_PLUGINS_WIDTH - 45, 32, WINDOWS_PLUGINS_WIDTH - 35, 51);
	gt_line (&window->_wnd->fb, window->_wnd->gc, WINDOWS_PLUGINS_WIDTH - 35, 52, WINDOWS_PLUGINS_WIDTH - 45, 72);
	
	font.size = 24;
	offset = (WINDOWS_PLUGINS_WIDTH / 2) - (font_width_str (&font, window->plugins[window->selected_item].desc, strlen (window->plugins[window->selected_item].desc)) / 2);
	font_draw_str (&font, window->plugins[window->selected_item].desc, strlen (window->plugins[window->selected_item].desc), &window->_wnd->fb, window->_wnd->gc, offset, 28);
	font.size = 16;
	offset = (WINDOWS_PLUGINS_WIDTH / 2) - (font_width_str (&font, window->plugins[window->selected_item].name, strlen (window->plugins[window->selected_item].name)) / 2);
	font_draw_str (&font, window->plugins[window->selected_item].name, strlen (window->plugins[window->selected_item].name), &window->_wnd->fb, window->_wnd->gc, offset, 60);
	
	wm_redraw (window->_wnd);
	wm_redraw (window->_wnd_footer);
}

static void window_plugins_show_config (w_plugins_t *window)
{
	int i;
	w_config_t mywindow;
	w_config_item_t items[8];
	
	w_config_value_t values_red[window->plugins_count+1];
	w_config_value_t values_green[window->plugins_count+1];
	w_config_value_t values_yellow[window->plugins_count+1];
	w_config_value_t values_blue[window->plugins_count+1];
	w_config_value_t values_long_red[window->plugins_count+1];
	w_config_value_t values_long_green[window->plugins_count+1];
	w_config_value_t values_long_yellow[window->plugins_count+1];
	w_config_value_t values_long_blue[window->plugins_count+1];
	
	// TODO: intl ()
	values_red[0].name = _malloc (8);
	values_green[0].name = _malloc (8);
	values_yellow[0].name = _malloc (8);
	values_blue[0].name = _malloc (8);
	values_long_red[0].name = _malloc (8);
	values_long_green[0].name = _malloc (8);
	values_long_yellow[0].name = _malloc (8);
	values_long_blue[0].name = _malloc (8);
	strcpy (values_red[0].name, "Nothing");
	strcpy (values_green[0].name, "Nothing");
	strcpy (values_yellow[0].name, "Nothing");
	strcpy (values_blue[0].name, "Nothing");
	strcpy (values_long_red[0].name, "Nothing");
	strcpy (values_long_green[0].name, "Nothing");
	strcpy (values_long_yellow[0].name, "Nothing");
	strcpy (values_long_blue[0].name, "Nothing");
	
	for (i=1; i<(window->plugins_count + 1); i++)
	{
		values_red[i].name = _malloc (strlen (window->plugins[i-1].name)+1);
		values_green[i].name = _malloc (strlen (window->plugins[i-1].name)+1);
		values_yellow[i].name = _malloc (strlen (window->plugins[i-1].name)+1);
		values_blue[i].name = _malloc (strlen (window->plugins[i-1].name)+1);
		values_long_red[i].name = _malloc (strlen (window->plugins[i-1].name)+1);
		values_long_green[i].name = _malloc (strlen (window->plugins[i-1].name)+1);
		values_long_yellow[i].name = _malloc (strlen (window->plugins[i-1].name)+1);
		values_long_blue[i].name = _malloc (strlen (window->plugins[i-1].name)+1);
		strcpy (values_red[i].name, window->plugins[i-1].name);
		strcpy (values_green[i].name, window->plugins[i-1].name);
		strcpy (values_yellow[i].name, window->plugins[i-1].name);
		strcpy (values_blue[i].name, window->plugins[i-1].name);
		strcpy (values_long_red[i].name, window->plugins[i-1].name);
		strcpy (values_long_green[i].name, window->plugins[i-1].name);
		strcpy (values_long_yellow[i].name, window->plugins[i-1].name);
		strcpy (values_long_blue[i].name, window->plugins[i-1].name);
	}
		
	
	items[0].type = 0;
	items[0].name = "Red button";
	items[0].values = values_red;
	items[0].values_count = window->plugins_count + 1;
	items[0].changed = false;
	if (window->red_item > -1)
		items[0].selected_value = window->red_item + 1;
	else
		items[0].selected_value = 0;

	items[1].type = 0;
	items[1].name = "Green button";
	items[1].values = values_green;
	items[1].values_count = window->plugins_count + 1;
	items[1].changed = false;
	if (window->green_item > -1)
		items[1].selected_value = window->green_item + 1;
	else
		items[1].selected_value = 0;

	items[2].type = 0;
	items[2].name = "Yellow button";
	items[2].values = values_yellow;
	items[2].values_count = window->plugins_count + 1;
	items[2].changed = false;
	if (window->yellow_item > -1)
		items[2].selected_value = window->yellow_item + 1;
	else
		items[2].selected_value = 0;

	items[3].type = 0;
	items[3].name = "Blue button";
	items[3].values = values_blue;
	items[3].values_count = window->plugins_count + 1;
	items[3].changed = false;
	if (window->blue_item > -1)
		items[3].selected_value = window->blue_item + 1;
	else
		items[3].selected_value = 0;

	items[4].type = 0;
	items[4].name = "Red button (long)";
	items[4].values = values_long_red;
	items[4].values_count = window->plugins_count + 1;
	items[4].changed = false;
	if (window->red_long_item > -1)
		items[4].selected_value = window->red_long_item + 1;
	else
		items[4].selected_value = 0;

	items[5].type = 0;
	items[5].name = "Green button (long)";
	items[5].values = values_long_green;
	items[5].values_count = window->plugins_count + 1;
	items[5].changed = false;
	if (window->green_long_item > -1)
		items[5].selected_value = window->green_long_item + 1;
	else
		items[5].selected_value = 0;

	items[6].type = 0;
	items[6].name = "Yellow button (long)";
	items[6].values = values_long_yellow;
	items[6].values_count = window->plugins_count + 1;
	items[6].changed = false;
	if (window->yellow_long_item > -1)
		items[6].selected_value = window->yellow_long_item + 1;
	else
		items[6].selected_value = 0;

	items[7].type = 0;
	items[7].name = "Blue button (long)";
	items[7].values = values_long_blue;
	items[7].values_count = window->plugins_count + 1;
	items[7].changed = false;
	if (window->blue_long_item > -1)
		items[7].selected_value = window->blue_long_item + 1;
	else
		items[7].selected_value = 0;
	
	/* draw config window */
	mywindow.title = intl (CONFIGURATION);
	mywindow.items = items;
	mywindow.items_count = 8;
	mywindow.selected_item = 0;
	mywindow.start_item = 0;
	window_config_init (&mywindow);
	window_config_update (&mywindow);
	window_config_show (&mywindow);
	window_config_execute (&mywindow);
	window_config_clean (&mywindow);
	
	if (mywindow.exit_ok)
	{
		window->red_item = items[0].selected_value - 1;
		window->green_item = items[1].selected_value - 1;
		window->yellow_item = items[2].selected_value - 1;
		window->blue_item = items[3].selected_value - 1;
		window->red_long_item = items[4].selected_value - 1;
		window->green_long_item = items[5].selected_value - 1;
		window->yellow_long_item = items[6].selected_value - 1;
		window->blue_long_item = items[7].selected_value - 1;
		config_set_launcher_red (window->plugins[items[0].selected_value - 1].filename);
		config_set_launcher_green (window->plugins[items[1].selected_value - 1].filename);
		config_set_launcher_yellow (window->plugins[items[2].selected_value - 1].filename);
		config_set_launcher_blue (window->plugins[items[3].selected_value - 1].filename);
		config_set_launcher_long_red (window->plugins[items[4].selected_value - 1].filename);
		config_set_launcher_long_green (window->plugins[items[5].selected_value - 1].filename);
		config_set_launcher_long_yellow (window->plugins[items[6].selected_value - 1].filename);
		config_set_launcher_long_blue (window->plugins[items[7].selected_value - 1].filename);
		
		if (config_save ()) log_add ("Configuration saved");
		else log_add ("Error saving configuration");
	}
	
	for (i=0; i<items[0].values_count; i++) _free (items[0].values[i].name);
	for (i=0; i<items[1].values_count; i++) _free (items[1].values[i].name);
	for (i=0; i<items[2].values_count; i++) _free (items[2].values[i].name);
	for (i=0; i<items[3].values_count; i++) _free (items[3].values[i].name);
	for (i=0; i<items[4].values_count; i++) _free (items[4].values[i].name);
	for (i=0; i<items[5].values_count; i++) _free (items[5].values[i].name);
	for (i=0; i<items[6].values_count; i++) _free (items[6].values[i].name);
	for (i=0; i<items[7].values_count; i++) _free (items[7].values[i].name);
}

static int window_plugins_event_handler (w_plugins_t *window, event_t *event)
{
	int key = event_key_2code (event);
	
	if (key == 0x18e)		// red button
	{
		if (event_key_2value (event) == event_key_value_up)
			return window->red_item;
		else if (event_key_2value (event) == event_key_value_rep)
			return window->red_long_item;
	}
	
	if (key == 0x18f)		// green button
	{
		if (event_key_2value (event) == event_key_value_up)
			return window->green_item;
		else if (event_key_2value (event) == event_key_value_rep)
			return window->green_long_item;
	}
	
	if (key == 0x190)		// yellow button
	{
		if (event_key_2value (event) == event_key_value_up)
			return window->yellow_item;
		else if (event_key_2value (event) == event_key_value_rep)
			return window->yellow_long_item;
	}
	
	if (key == 0x191)		// blue button
	{
		if (event_key_2value (event) == event_key_value_up)
			return window->blue_item;
		else if (event_key_2value (event) == event_key_value_rep)
			return window->blue_long_item;
	}
	
	if (event_key_2value (event) == event_key_value_down)
		return -1;
	
	if (key == KEY_RIGHT)
	{
		window->selected_item++;
		if (window->selected_item >= window->plugins_count) window->selected_item = 0;
	}
		
	if (key == KEY_LEFT)
	{
		window->selected_item--;
		if (window->selected_item < 0) window->selected_item = window->plugins_count - 1;
	}
	
	if (key == 0x160)		// ok button
	{
		window->exit_ok = true;
		return window->selected_item;
	}
	
	if (key == 0x8b)		// menu button
		window_plugins_show_config (window);
	
	if (key == KEY_BACK)
	{
		window->exit_ok = false;
		return -2;
	}
	
	return -1;
}

static int window_plugins_execute (w_plugins_t *window)
{
	event_t event;
	int status;
	
	plug_event_init ();
	while (true)
	{
		if (plug_event_rcv (&event) < 0)
			continue;
		
		if (event.type != event_type_key)
			continue;
		
		plug_event_close ();
		status = window_plugins_event_handler (window, &event);
		if ((status == -2) || (status >= 0))
			break;
			
		window_plugins_update (window);
		
		plug_event_init ();
	}
	return status;
}
