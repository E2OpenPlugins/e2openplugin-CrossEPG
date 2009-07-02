#define WINDOWS_CONFIG_WIDTH 300
#define WINDOWS_CONFIG_HEIGHT 430
static img_t config_background = NULL;

typedef struct w_config_value_s
{
	int		id;
	char	*name;
	bool	selected;
} w_config_value_t;

typedef struct w_config_item_s
{
	int					type;				// 0 = list | 1 = int | 2 = checklist
	char				*name;
	w_config_value_t	*values;			// an array of values
	int					values_count;		// array length
	int					selected_value;		// array index
	int					min;
	int					max;
	int					step;
	bool				changed;
} w_config_item_t;

typedef struct w_config_s
{
	char			*title;
	w_config_item_t	*items;
	int				items_count;
	int				selected_item;
	int				start_item;
	bool			exit_ok;
	window_t		*_wnd;
} w_config_t;

static void window_config_init (w_config_t *window)
{
	char file[256];
	rect_t rect = { 210, 70, WINDOWS_CONFIG_WIDTH, WINDOWS_CONFIG_HEIGHT };
	window->_wnd = wm_create (&rect, window->title, NULL, NULL, COLOR_TRANSPARENT);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "backgrounds/config.bmp");
	if (config_background) img_close (config_background);
	config_background = img_open_absolute_path (file);
}

static void window_config_show (w_config_t *window)
{
	wm_show (window->_wnd);	
}

static void window_config_clean (w_config_t *window)
{
	wm_destroy (window->_wnd);
	if (config_background) img_close (config_background);
	config_background = NULL;
}

static void window_config_draw_item (window_t *window, w_config_item_t *item, int row, bool selected)
{
	char value[256];
	struct _font font;
	font.size = 17;
	if (selected)
	{
		gc_set_fc (window->gc, COLOR_CONFIG_SELECTED_BACKGROUND);
		gt_fillrect (&window->fb, window->gc, 15, 60 + (60*row), WINDOWS_CONFIG_WIDTH - 30, 50);
		gc_set_fc (window->gc, COLOR_CONFIG_SELECTED_FOREGROUND);
		gc_set_bc (window->gc, COLOR_CONFIG_SELECTED_BACKGROUND);
		gc_set_lt (window->gc, 2);
		gt_line (&window->fb, window->gc, 35,  70 + (60*row), 25, 84 + (60*row));
		gt_line (&window->fb, window->gc, 25,  86 + (60*row), 35, 100 + (60*row));
		gt_line (&window->fb, window->gc, WINDOWS_CONFIG_WIDTH - 35, 70 + (60*row), WINDOWS_CONFIG_WIDTH - 25, 84 + (60*row));
		gt_line (&window->fb, window->gc, WINDOWS_CONFIG_WIDTH - 25, 86 + (60*row), WINDOWS_CONFIG_WIDTH - 35, 100 + (60*row));
	}
	else
	{
		gc_set_fc (window->gc, COLOR_CONFIG_FOREGROUND);
		gc_set_bc (window->gc, COLOR_CONFIG_BACKGROUND);
	}
	
	int offset = (WINDOWS_CONFIG_WIDTH / 2) - (font_width_str (&font, item->name, strlen (item->name)) / 2);
	font_draw_str (&font, item->name, strlen (item->name), &window->fb, window->gc, offset, 63 + (60*row));
	
	if (item->type == 0)
		strcpy (value, item->values[item->selected_value].name);
	else if (item->type == 1)
		sprintf (value, "%d", item->selected_value);
	else if (item->type == 2)
		sprintf (value, "%s (%s)", item->values[item->selected_value].name, (item->values[item->selected_value].selected)?intl (YES):intl (NO));
	else
		return;
	
	font.size = 14;
	offset = (WINDOWS_CONFIG_WIDTH / 2) - (font_width_str (&font, value, strlen (value)) / 2);
	font_draw_str (&font, value, strlen (value), &window->fb, window->gc, offset, 88 + (60*row));	
}

static void window_config_update (w_config_t *window)
{
	int i, offset;
	struct _font font;
	
	font.size = 17;
	
	if (config_background)
	{
		gc_set_fc (window->_wnd->gc, COLOR_TRANSPARENT);	
		gt_fillrect (&window->_wnd->fb, window->_wnd->gc, 0, 0, WINDOWS_CONFIG_WIDTH, WINDOWS_CONFIG_HEIGHT);
		img_draw (&window->_wnd->fb, config_background, window->_wnd->gc, 0, 0, WINDOWS_CONFIG_WIDTH, WINDOWS_CONFIG_HEIGHT, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
	}
	else
	{
		gc_set_fc (window->_wnd->gc, COLOR_CONFIG_BACKGROUND);	
		gt_fillrect (&window->_wnd->fb, window->_wnd->gc, 0, 0, WINDOWS_CONFIG_WIDTH, WINDOWS_CONFIG_HEIGHT);
		gc_set_fc (window->_wnd->gc, COLOR_CONFIG_BORDER);	
		gc_set_lt (window->_wnd->gc, 2);
		gt_rect (&window->_wnd->fb, window->_wnd->gc, 1, 1, WINDOWS_CONFIG_WIDTH - 2, WINDOWS_CONFIG_HEIGHT - 2);
	}
	
	gc_set_fc (window->_wnd->gc, COLOR_CONFIG_FOREGROUND);
	gc_set_bc (window->_wnd->gc, COLOR_CONFIG_BACKGROUND);
	
	offset = (WINDOWS_CONFIG_WIDTH / 2) - (font_width_str (&font, window->title, strlen (window->title)) / 2);
	font_draw_str (&font, window->title, strlen (window->title), &window->_wnd->fb, window->_wnd->gc, offset, 8);

	//gc_set_fc (window->_wnd->gc, COLOR_CONFIG_BORDER);	
	//gt_rect (&window->_wnd->fb, window->_wnd->gc, 1, 32, WINDOWS_CONFIG_WIDTH - 2, 1);

	for (i=0; i<5; i++)
	{
		if ((window->start_item+i) < window->items_count)
			window_config_draw_item (window->_wnd, &window->items[window->start_item+i], i, ((window->start_item + i) == window->selected_item));
	}

	offset = 10;
	if (button_ok)
		img_draw (&window->_wnd->fb, button_ok, window->_wnd->gc, WINDOWS_CONFIG_WIDTH - 40, WINDOWS_CONFIG_HEIGHT - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);

	if (button_exit)
		img_draw (&window->_wnd->fb, button_exit, window->_wnd->gc, WINDOWS_CONFIG_WIDTH - 80, WINDOWS_CONFIG_HEIGHT - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);

	if ((button_v) && (window->items[window->selected_item].type == 2))
		img_draw (&window->_wnd->fb, button_v, window->_wnd->gc, WINDOWS_CONFIG_WIDTH - 120, WINDOWS_CONFIG_HEIGHT - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);

	wm_redraw (window->_wnd);
}

static int window_config_event_handler (w_config_t *window, event_t *event)
{
	if (event_key_2value (event) == event_key_value_up)
		return 0;

	int key = event_key_2code (event);
	//log_add ("key: %x", key);
	
	if (key == KEY_DOWN)
	{
		window->selected_item++;
		if (window->selected_item >= window->items_count)
		{
			window->selected_item = 0;
			window->start_item = 0;
		}
		if (window->start_item > window->selected_item) window->start_item--;
		if (window->start_item < (window->selected_item - 4)) window->start_item++;
	}
		
	if (key == KEY_UP)
	{
		window->selected_item--;
		if (window->selected_item < 0)
		{
			window->selected_item = window->items_count - 1;
			if (window->items_count > 5) window->start_item = window->selected_item - 4;
		}
		if (window->start_item > window->selected_item) window->start_item--;
		if (window->start_item < (window->selected_item - 4)) window->start_item++;
	}
	if (key == KEY_LEFT)
	{
		w_config_item_t *item = &window->items[window->selected_item];
		if ((item->type == 0) || (item->type == 2))
		{
			item->selected_value--;
			if (item->selected_value < 0) item->selected_value = item->values_count - 1;
			item->changed = true;
		}
		else if (item->type == 1)
		{
			item->selected_value -= item->step;
			if (item->selected_value < item->min) item->selected_value = item->max;
			item->changed = true;
		}
	}

	if (key == KEY_RIGHT)
	{
		w_config_item_t *item = &window->items[window->selected_item];
		if ((item->type == 0) || (item->type == 2))
		{
			item->selected_value++;
			if (item->selected_value >= item->values_count) item->selected_value = 0;
		}
		else if (item->type == 1)
		{
			item->selected_value += item->step;
			if (item->selected_value > item->max) item->selected_value = item->min;
		}
	}
	
	if ((key == 0x3f) || (key == 0x191))		// √ button or blue button (91hd)
	{
		w_config_item_t *item = &window->items[window->selected_item];
		if (item->type == 2)
		{
			w_config_value_t *value = &item->values[item->selected_value];
			value->selected = !value->selected;
		}
	}

	if (key == 0x160)		// ok button
	{
		window->exit_ok = true;
		return -1;
	}

	if (key == KEY_BACK)
	{
		window->exit_ok = false;
		return -1;
	}
	
	return 0;
}

static void window_config_execute (w_config_t *window)
{
	event_t event;
	
	plug_event_init ();
	while (true)
	{
		if (plug_event_rcv (&event) < 0)
			continue;

		if (event.type != event_type_key)
			continue;

		plug_event_close ();
		if (window_config_event_handler (window, &event) < 0)
			break;
			
		window_config_update (window);
		
		plug_event_init ();
	}
}
