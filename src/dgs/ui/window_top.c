static window_t *window_top = NULL;

static void window_top_update ()
{
	struct _font font;
	time_t now = time (NULL);
	struct tm loctime;
	char time_string[10];
	int width;
	
	gc_set_fc (window_top->gc, COLOR_TRANSPARENT);
	gt_fillrect (&window_top->gui, window_top->gc, 0, 0, 1280, 60);
	
	gc_set_bc (window_top->gc, COLOR_TRANSPARENT);
	
	localtime_r (&now, &loctime);
	
	font.size = 18;
	
	if (grid_icon)
		gui_draw_image (&window_top->gui, grid_icon, window_top->gc, 35, 25, 40, 36, 0); 

	if (list_icon)
		gui_draw_image (&window_top->gui, list_icon, window_top->gc, 276, 25, 40, 36, 0); 

	if (light_icon)
		gui_draw_image (&window_top->gui, light_icon, window_top->gc, 532, 25, 40, 36, 0); 

	if (scheduler_icon)
		gui_draw_image (&window_top->gui, scheduler_icon, window_top->gc, 788, 25, 40, 36, 0); 

	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_RED_TAB);
	font_draw_str (&font, intl (GRID), strlen (intl (GRID)), &window_top->gui, window_top->gc, 86, 32);
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_GREEN_TAB);
	font_draw_str (&font, intl (LIST), strlen (intl (LIST)), &window_top->gui, window_top->gc, 327, 32);
	width = font_width_str (&font, intl (LIGHT), strlen (intl (LIGHT)));
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_YELLOW_TAB);
	font_draw_str (&font, intl (LIGHT), strlen (intl (LIGHT)), &window_top->gui, window_top->gc, 583, 32);
	width = font_width_str (&font, intl (SCHEDULER), strlen (intl (SCHEDULER)));
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_BLUE_TAB);
	font_draw_str (&font, intl (SCHEDULER), strlen (intl (SCHEDULER)), &window_top->gui, window_top->gc, 839, 32);
	
	font.size = 25;
	strftime (time_string, 10, intl (TIME_HM), &loctime);
	width = font_width_str (&font, time_string, strlen (time_string));
	gc_set_fc (window_top->gc, COLOR_WINDOW_TOP_CLOCK);
	font_draw_str (&font, time_string, strlen (time_string), &window_top->gui, window_top->gc, 1220-width, 25);
	
	wm_redraw (window_top);
}

static void window_top_show ()
{
	if (window_top) window_top_update ();
	else
	{
		rect_t rect = { 0, 0, 1280, 60 };
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
