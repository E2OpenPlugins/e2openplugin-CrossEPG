static window_t *window_progress = NULL;
static gui_image_t progress_background = NULL;
//static int window_progress_x = 0;
//static int window_progress_y = 0;
static int window_progress_show_bar = true;

void window_progress_set_pos (int x, int y)
{
	//window_progress_x = x;
	//window_progress_y = y;
}

void window_progress_set_show_bar (bool value)
{
	window_progress_show_bar = value;
}

static void window_progress_init ()
{
	char file[256];
	rect_t rect = { 0, 0, 1280, 720 };
	window_progress = wm_create (&rect, "CrossEPG Progress", NULL, NULL, COLOR_TRANSPARENT);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "backgrounds/progress.png");
	if (progress_background) gui_close_image (progress_background);
	progress_background = gui_open_image_by_path(file, gui_image_type_png);
}

static void window_progress_show ()
{
	wm_show (window_progress);	
}

static void window_progress_clean ()
{
	wm_destroy (window_progress);
	if (progress_background) gui_close_image (progress_background);
	progress_background = NULL;
}

static void window_progress_update (char *message1, char *message2, int value)
{
	struct _font font;
	
	font.size = 16;
	
	if (progress_background)
	{
		gc_set_fc (window_progress->gc, COLOR_TRANSPARENT);
		gt_fillrect (&window_progress->gui, window_progress->gc, 0, 0, 1280, 720);
		gui_draw_image (&window_progress->gui, progress_background, window_progress->gc, 0, 0, 1280, 720, 0); 
	}
	
	gc_set_fc (window_progress->gc, COLOR_PROGRESS_FOREGROUND);	
	gc_set_bc (window_progress->gc, COLOR_PROGRESS_BACKGROUND);	
	
	if (window_progress_show_bar)
	{
		font_draw_str (&font, message1, strlen (message1), &window_progress->gui, window_progress->gc, 50, 620);
		font_draw_str (&font, message2, strlen (message2), &window_progress->gui, window_progress->gc, 50, 640);
		
		gt_fillrect (&window_progress->gui, window_progress->gc, 0, 604, value * 12.8, 2);
	}
	else
	{
		font_draw_str (&font, message1, strlen (message1), &window_progress->gui, window_progress->gc, 50, 620);
		if (strlen (message2) > 0)
			font_draw_str (&font, message2, strlen (message2), &window_progress->gui, window_progress->gc, 50, 640);
	}

	wm_redraw (window_progress);
}
