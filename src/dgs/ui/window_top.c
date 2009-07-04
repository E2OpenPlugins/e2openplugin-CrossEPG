static window_t *window_top = NULL;

static void window_top_update ()
{
	struct _font font;
	time_t now = time (NULL);
	struct tm loctime;
	char time_string[10];
	int width;
	
	gc_set_fc (window_top->gc, COLOR_TRANSPARENT);
	gt_fillrect (&window_top->fb, window_top->gc, 0, 0, 720, 50);
	
	gc_set_bc (window_top->gc, COLOR_TRANSPARENT);
	
	localtime_r (&now, &loctime);
	
	font.size = 17;
	strftime (time_string, 10, intl (TIME_HM), &loctime);
	width = font_width_str (&font, time_string, strlen (time_string));
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_CLOCK);
	font_draw_str (&font, time_string, strlen (time_string), &window_top->fb, window_top->gc, 670-width, 26);
	width = font_width_str (&font, intl (GRID), strlen (intl (GRID)));
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_RED_TAB);
	font_draw_str (&font, intl (GRID), strlen (intl (GRID)), &window_top->fb, window_top->gc, 75-(width/2), 26);
	width = font_width_str (&font, intl (LIST), strlen (intl (LIST)));
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_GREEN_TAB);
	font_draw_str (&font, intl (LIST), strlen (intl (LIST)), &window_top->fb, window_top->gc, 225-(width/2), 26);
	width = font_width_str (&font, intl (LIGHT), strlen (intl (LIGHT)));
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_YELLOW_TAB);
	font_draw_str (&font, intl (LIGHT), strlen (intl (LIGHT)), &window_top->fb, window_top->gc, 375-(width/2), 26);
	width = font_width_str (&font, intl (SCHEDULER), strlen (intl (SCHEDULER)));
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_BLUE_TAB);
	font_draw_str (&font, intl (SCHEDULER), strlen (intl (SCHEDULER)), &window_top->fb, window_top->gc, 525-(width/2), 26);
	
	wm_redraw (window_top);
}

static void window_top_show ()
{
	if (window_top) window_top_update ();
	else
	{
		rect_t rect = { 0, 0, 720, 50 };
		window_top = wm_create (&rect, "", NULL, NULL, COLOR_TRANSPARENT);
		window_top_update ();
		wm_show (window_top);
	}
}

static void window_top_hide ()
{
	if (window_top) wm_destroy (window_top);
	window_top = NULL;
}
