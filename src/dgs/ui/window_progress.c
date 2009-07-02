static window_t *window_progress = NULL;
static img_t progress_background = NULL;
static int window_progress_x = 440;
static int window_progress_y = 440;
static int window_progress_show_bar = true;

void window_progress_set_pos (int x, int y)
{
	window_progress_x = x;
	window_progress_y = y;
}

void window_progress_set_show_bar (bool value)
{
	window_progress_show_bar = value;
}

static void window_progress_init ()
{
	char file[256];
	rect_t rect = { window_progress_x, window_progress_y, 240, 96 };
	window_progress = wm_create (&rect, "CrossEPG Progress", NULL, NULL, COLOR_TRANSPARENT);
	sprintf (file, "%s/skins/%s/%s", config_get_home_directory (), config_get_skin (), "backgrounds/progress.bmp");
	if (progress_background) img_close (progress_background);
	progress_background = img_open_absolute_path (file);
}

static void window_progress_show ()
{
	wm_show (window_progress);	
}

static void window_progress_clean ()
{
	wm_destroy (window_progress);
	if (progress_background) img_close (progress_background);
	progress_background = NULL;
}

static void window_progress_update (char *message1, char *message2, int value)
{
	struct _font font;
	
	font.size = 13;
	
	if (progress_background)
	{
		gc_set_fc (window_progress->gc, COLOR_TRANSPARENT);	
		gt_fillrect (&window_progress->fb, window_progress->gc, 0, 0, 240, 96);
		img_draw (&window_progress->fb, progress_background, window_progress->gc, 0, 0, 240, 96, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
	}
	else
	{
		gc_set_fc (window_progress->gc, COLOR_PROGRESS_BACKGROUND);	
		gt_fillrect (&window_progress->fb, window_progress->gc, 0, 0, 240, 96);
		gc_set_fc (window_progress->gc, COLOR_PROGRESS_BORDER);	
		gc_set_lt (window_progress->gc, 2);
		gt_rect (&window_progress->fb, window_progress->gc, 1, 1, 239, 94);
	}
	
	gc_set_fc (window_progress->gc, COLOR_PROGRESS_FOREGROUND);	
	gc_set_bc (window_progress->gc, COLOR_PROGRESS_BACKGROUND);	
	
	if (window_progress_show_bar)
	{
		int width = font_width_str (&font, message1, strlen (message1));
		font_draw_str (&font, message1, strlen (message1), &window_progress->fb, window_progress->gc, (240 - width) / 2, 13);
		width = font_width_str (&font, message2, strlen (message2));
		font_draw_str (&font, message2, strlen (message2), &window_progress->fb, window_progress->gc, (240 - width) / 2, 33);
		
		gt_rect (&window_progress->fb, window_progress->gc, 10, 61, 220, 20);
		gt_fillrect (&window_progress->fb, window_progress->gc, 10, 61, value * 2.2, 20);
	}
	else
	{
		if (strlen (message2) > 0)
		{
			int width = font_width_str (&font, message1, strlen (message1));
			font_draw_str (&font, message1, strlen (message1), &window_progress->fb, window_progress->gc, (240 - width) / 2, 23);
			width = font_width_str (&font, message2, strlen (message2));
			font_draw_str (&font, message2, strlen (message2), &window_progress->fb, window_progress->gc, (240 - width) / 2, 53);
		}
		else
		{
			int width = font_width_str (&font, message1, strlen (message1));
			font_draw_str (&font, message1, strlen (message1), &window_progress->fb, window_progress->gc, (240 - width) / 2, 38);
		}
	}

	wm_redraw (window_progress);
}
