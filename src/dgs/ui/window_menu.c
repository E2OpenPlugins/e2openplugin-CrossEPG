#define WINDOWS_MENU_WIDTH 300
#define WINDOWS_MENU_HEIGHT 450
static gui_image_t menu_background = NULL;

typedef struct w_menu_s
{
	char			*title;
	char			**items;
	int				items_count;
	int				selected_item;
	bool			exit_ok;
	window_t		*_wnd;
} w_menu_t;

static void window_menu_init (w_menu_t *window)
{
	char file[256];
	rect_t rect = { 490, 135, WINDOWS_MENU_WIDTH, WINDOWS_MENU_HEIGHT };
	window->_wnd = wm_create (&rect, window->title, NULL, NULL, COLOR_TRANSPARENT);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "backgrounds/menu.png");
	if (menu_background) gui_close_image (menu_background);
	menu_background = gui_open_image_by_path(file, gui_image_type_png);
}

static void window_menu_show (w_menu_t *window)
{
	wm_show (window->_wnd);	
}

static void window_menu_clean (w_menu_t *window)
{
	wm_destroy (window->_wnd);
	if (menu_background) gui_close_image (menu_background);
	menu_background = NULL;
}

static void window_menu_draw_item (window_t *window, char *item, int row, bool selected)
{
	struct _font font;
	font.size = 19;
	if (selected)
	{
		gc_set_fc (window->gc, COLOR_MENU_SELECTED_BACKGROUND);
		gt_fillrect (&window->gui, window->gc, 15, 70 + (38*row), WINDOWS_MENU_WIDTH - 30, 28);
		gc_set_fc (window->gc, COLOR_MENU_SELECTED_FOREGROUND);
		gc_set_bc (window->gc, COLOR_MENU_SELECTED_BACKGROUND);
	}
	else
	{
		gc_set_fc (window->gc, COLOR_MENU_FOREGROUND);
		gc_set_bc (window->gc, COLOR_MENU_BACKGROUND);
	}
	int offset = (WINDOWS_MENU_WIDTH / 2) - (font_width_str (&font, item, strlen (item)) / 2);
	font_draw_str (&font, item, strlen (item), &window->gui, window->gc, offset, 73 + (38*row));	
}

static void window_menu_update (w_menu_t *window)
{
	int i, offset;
	struct _font font;
	
	font.size = 22;
	
	if (menu_background)
	{
		gc_set_fc (window->_wnd->gc, COLOR_TRANSPARENT);	
		gt_fillrect (&window->_wnd->gui, window->_wnd->gc, 0, 0, WINDOWS_MENU_WIDTH, WINDOWS_MENU_HEIGHT);
		gui_draw_image (&window->_wnd->gui, menu_background, window->_wnd->gc, 0, 0, WINDOWS_MENU_WIDTH, WINDOWS_MENU_HEIGHT, 0);
		//img_draw (&window->_wnd->gui, menu_background, window->_wnd->gc, 0, 0, WINDOWS_MENU_WIDTH, WINDOWS_MENU_HEIGHT, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
	}
	else
	{
		gc_set_fc (window->_wnd->gc, COLOR_MENU_BACKGROUND);	
		gt_fillrect (&window->_wnd->gui, window->_wnd->gc, 0, 0, WINDOWS_MENU_WIDTH, WINDOWS_MENU_HEIGHT);
		gc_set_fc (window->_wnd->gc, COLOR_MENU_BORDER);	
		gc_set_lt (window->_wnd->gc, 2);
		gt_rect (&window->_wnd->gui, window->_wnd->gc, 1, 1, WINDOWS_MENU_WIDTH - 2, WINDOWS_MENU_HEIGHT - 2);
	}
	
	gc_set_fc (window->_wnd->gc, COLOR_MENU_FOREGROUND);
	gc_set_bc (window->_wnd->gc, COLOR_MENU_BACKGROUND);
	
	//offset = (WINDOWS_MENU_WIDTH / 2) - (font_width_str (&font, window->title, strlen (window->title)) / 2);
	font_draw_str (&font, window->title, strlen (window->title), &window->_wnd->gui, window->_wnd->gc, 55, 16);

	//gc_set_fc (window->_wnd->gc, COLOR_MENU_BORDER);	
	//gt_rect (&window->_wnd->gui, window->_wnd->gc, 1, 32, WINDOWS_MENU_WIDTH - 2, 1);

	for (i=0; i<window->items_count; i++)
		window_menu_draw_item (window->_wnd, window->items[i], i, (i == window->selected_item));

	offset = 10;
	if (button_ok)
		gui_draw_image (&window->_wnd->gui, button_ok, window->_wnd->gc, WINDOWS_MENU_WIDTH - 50, WINDOWS_MENU_HEIGHT - 40, 26, 23, 0);
		//img_draw (&window->_wnd->gui, button_ok, window->_wnd->gc, WINDOWS_MENU_WIDTH - 40, WINDOWS_MENU_HEIGHT - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);

	if (button_exit)
		gui_draw_image (&window->_wnd->gui, button_exit, window->_wnd->gc, WINDOWS_MENU_WIDTH - 90, WINDOWS_MENU_HEIGHT - 40, 26, 23, 0);
		//img_draw (&window->_wnd->gui, button_exit, window->_wnd->gc, WINDOWS_MENU_WIDTH - 80, WINDOWS_MENU_HEIGHT - 30, 30, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);

	wm_redraw (window->_wnd);
}

static int window_menu_event_handler (w_menu_t *window, event_t *event)
{
	if (event_key_2value (event) == event_key_value_up)
		return 0;

	int key = event_key_2code (event);
	
	if (key == KEY_DOWN)
	{
		window->selected_item++;
		if (window->selected_item >= window->items_count) window->selected_item = 0;
	}
		
	if (key == KEY_UP)
	{
		window->selected_item--;
		if (window->selected_item < 0) window->selected_item = window->items_count - 1;
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

static void window_menu_execute (w_menu_t *window)
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
		if (window_menu_event_handler (window, &event) < 0)
			break;
			
		window_menu_update (window);
		
		plug_event_init ();
	}
}
