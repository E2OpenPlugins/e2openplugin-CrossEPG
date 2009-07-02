typedef struct w_box_s
{
	char			*title;
	char			*message;
	bool			exit_ok;
	int				type;
	window_t		*_wnd;
	int				x;
	int				y;
	int				width;
	int				height;
	int				offset_rows;
	bool			more_rows;
	img_t			background;
	char			*background_file;
} w_box_t;

static void window_box_init (w_box_t *window)
{
	char file[256];
	rect_t rect = { window->x, window->y, window->width, window->height };
	window->_wnd = wm_create (&rect, window->title, NULL, NULL, COLOR_TRANSPARENT);
	sprintf (file, "%s/skins/%s/backgrounds/%s", config_get_home_directory (), config_get_skin (), window->background_file);
	window->background = img_open_absolute_path (file);
}

static void window_box_show (w_box_t *window)
{
	wm_show (window->_wnd);	
}

static void window_box_clean (w_box_t *window)
{
	wm_destroy (window->_wnd);
	if (window->background) img_close (window->background);
}

static void window_box_update (w_box_t *window)
{
	int offset;
	struct _font font;
	font.size = 17;
	
	if (window->background)
	{
		gc_set_fc (window->_wnd->gc, COLOR_TRANSPARENT);	
		gt_fillrect (&window->_wnd->fb, window->_wnd->gc, 0, 0, window->width, window->height);
		img_draw (&window->_wnd->fb, window->background, window->_wnd->gc, 0, 0, window->width, window->height, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
	}
	else
	{
		gc_set_fc (window->_wnd->gc, COLOR_BOX_BACKGROUND);	
		gt_fillrect (&window->_wnd->fb, window->_wnd->gc, 0, 0, window->width, window->height);
		gc_set_fc (window->_wnd->gc, COLOR_BOX_BORDER);	
		gc_set_lt (window->_wnd->gc, 2);
		gt_rect (&window->_wnd->fb, window->_wnd->gc, 1, 1, window->width - 2, window->height - 2);
	}
	
	gc_set_fc (window->_wnd->gc, COLOR_BOX_FOREGROUND);
	gc_set_bc (window->_wnd->gc, COLOR_BOX_BACKGROUND);
	
	offset = (window->width / 2) - (font_width_str (&font, window->title, strlen (window->title)) / 2);
	font_draw_str (&font, window->title, strlen (window->title), &window->_wnd->fb, window->_wnd->gc, offset, 8);
	
	//gc_set_fc (window->_wnd->gc, COLOR_BOX_BORDER);	
	//gt_rect (&window->_wnd->fb, window->_wnd->gc, 1, 32, window->width - 2, 1);
	
	gc_set_fc (window->_wnd->gc, COLOR_BOX_FOREGROUND);
	window->more_rows = !textarea (window->_wnd, window->message, 10, 45, window->width - 20, 0, font.size, 24, 14, window->offset_rows);
	
	font.size = 12;
	
	if (window->type == 1)
	{
		if (button_ok)
			img_draw (&window->_wnd->fb, button_ok, window->_wnd->gc, window->width - 40, window->height - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
		else
			font_draw_str (&font, "Ok", 2, &window->_wnd->fb, window->_wnd->gc, window->width - 40, window->height - 30);
			
		if (button_exit)
			img_draw (&window->_wnd->fb, button_exit, window->_wnd->gc, window->width - 80, window->height - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
		else
			font_draw_str (&font, "Exit", 4, &window->_wnd->fb, window->_wnd->gc, window->width - 80, window->height - 30);
	}
	else if (window->type == 2)
	{
		if (button_i)
			img_draw (&window->_wnd->fb, button_i, window->_wnd->gc, window->width - 40, window->height - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
		else
			font_draw_str (&font, "i", 4, &window->_wnd->fb, window->_wnd->gc, window->width - 40, window->height - 30);
	}
	else
	{
		if (button_exit)
			img_draw (&window->_wnd->fb, button_exit, window->_wnd->gc, window->width - 40, window->height - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
		else
			font_draw_str (&font, "Exit", 4, &window->_wnd->fb, window->_wnd->gc, window->width - 40, window->height - 30);
	}
	
	wm_redraw (window->_wnd);
}

static int window_box_event_handler (w_box_t *window, event_t *event)
{
	if (event_key_2value (event) == event_key_value_up)
		return 0;
	
	int key = event_key_2code (event);
	
	if ((key == 0xbb) || key == KEY_UP)		// vol up
		if (window->offset_rows > 0) window->offset_rows--;

	if ((key == 0xbc) || key == KEY_DOWN)		// vol down
		if (window->more_rows) window->offset_rows++;

	if (window->type == 1)
	{
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
	}
	if (window->type == 2)
	{
		if ((key == 0x166) || (key == KEY_BACK))		// i
		{
			window->exit_ok = true;
			return -1;
		}
	}
	else
	{
		if (key == KEY_BACK)
		{
			window->exit_ok = true;
			return -1;
		}
	}
		
	return 0;
}

static void window_box_execute (w_box_t *window)
{
	event_t event;
	
	// sleep (1);
	plug_event_init ();
	while (true)
	{
		if (plug_event_rcv (&event) < 0)
			continue;

		if (event.type != event_type_key)
			continue;

		plug_event_close ();
		if (window_box_event_handler (window, &event) < 0)
			break;
			
		window_box_update (window);
		
		plug_event_init ();
	}
}

int show_message_box_with_pos (char *title, char* message, int type, int x, int y, int width, int height, char *background)
{
	w_box_t wbox;
	wbox.title = title;
	wbox.message = message;
	wbox.type = type;
	wbox.x = x;
	wbox.y = y;
	wbox.width = width;
	wbox.height = height;
	wbox.offset_rows = 0;
	wbox.more_rows = false;
	wbox.background_file = background;
	
	window_box_init (&wbox);
	window_box_update (&wbox);
	window_box_show (&wbox);
	window_box_execute (&wbox);
	window_box_clean (&wbox);
	
	if (wbox.exit_ok) return 0;
	else return -1;
}

int show_message_box (char *title, char* message, int type)
{
	return show_message_box_with_pos (title, message, type, 160, 100, 400, 150, "info_small.bmp");
}
