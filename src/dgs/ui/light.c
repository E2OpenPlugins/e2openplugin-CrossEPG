static window_t *window_light = NULL;

#define LIGHT_X_OFFSET 	35
#define LIGHT_Y_OFFSET 	600
#define LIGHT_WIDTH		1195
#define LIGHT_HEIGHT	120

static void light_update_show_title (epgdb_title_t *title, int row)
{
	struct tm loctime;
	struct tm loctime_now;
	char start_time[10];
	char missing[256];
	char missing2[256];
	time_t tmp_time = dgs_helper_adjust_daylight (title->start_time);
	time_t now = time (NULL) - (dgs_helper_get_daylight_saving () * (60 * 60));
	int offset;
	localtime_r (&now, &loctime_now);
	localtime_r (&tmp_time, &loctime);
	struct _font font;
	int sch_mode = -1;
	scheduler_t *scheduler = scheduler_get_by_channel_and_title (selected_channel, title);
	if (scheduler != NULL) sch_mode = scheduler->type;
	
	if (sch_mode == 0) gc_set_fc (window_light->gc, COLOR_LIGHT_REC_FOREGROUND);
	else if (sch_mode == 1) gc_set_fc (window_light->gc, COLOR_LIGHT_ZAP_FOREGROUND);
	else gc_set_fc (window_light->gc, COLOR_LIGHT_FOREGROUND);
	
	font.size = 19;
	
	if ((loctime.tm_year == loctime_now.tm_year) && (loctime.tm_mon == loctime_now.tm_mon) && (loctime.tm_mday == loctime_now.tm_mday))
	{
		offset = font_width_str (&font, intl (TODAY), strlen (intl (TODAY)));
		font_draw_str (&font, intl (TODAY), strlen (intl (TODAY)), &window_light->gui, window_light->gc, 0, 48+(row*29));
	}
	else
	{
		offset = font_width_str (&font, intl (SUNDAY + loctime.tm_wday), strlen (intl (SUNDAY + loctime.tm_wday)));
		font_draw_str (&font, intl (SUNDAY + loctime.tm_wday), strlen (intl (SUNDAY + loctime.tm_wday)), &window_light->gui, window_light->gc, 0, 48+(row*29));
	}
	
	font.size = 17;
		
	strftime (start_time, 255, intl (TIME_HM), &loctime);
	
	font_draw_str (&font, start_time, strlen (start_time), &window_light->gui, window_light->gc, offset+8, 48+(row*29));
	char *name = epgdb_read_description (title);
	font_draw_str (&font, name, strlen (name), &window_light->gui, window_light->gc, offset+70, 48+(row*29));
	offset += (70 + font_width_str (&font, name, strlen (name)));
	_free (name);
	
	now = time (NULL);	
	if ((dgs_helper_adjust_daylight (title->start_time) + title->length) < now)
	{
		// elapsed
		sprintf (missing, intl (ELAPSED));
	}
	else if (dgs_helper_adjust_daylight (title->start_time) > now)
	{
		// in the future
		int minutes = (dgs_helper_adjust_daylight (title->start_time) - now) / 60;
		int hours = minutes / 60;
		int days = hours / 24;
		minutes = minutes % 60;
		hours = hours % 24;
		if (days > 0)
			sprintf (missing, intl (MISSING1), days, hours, minutes);
		else if (hours > 0)
			sprintf (missing, intl (MISSING2), hours, minutes);
		else
			sprintf (missing, intl (MISSING3), minutes);
	}
	else
	{
		// now
		sprintf (missing, intl (STARTED), (int)(now - dgs_helper_adjust_daylight (title->start_time)) / 60);
	}
	
	char *text = ui_resize_string (missing, font.size, 570 - offset);
	sprintf (missing2, "(%s)", text);
	font_draw_str (&font, missing2, strlen (missing2), &window_light->gui, window_light->gc, offset+12, 48+(row*29));
	_free (text);
}

static void light_update ()
{
	struct _font font;
	gc_set_fc (window_light->gc, COLOR_LIGHT_BACKGROUND);
	gt_fillrect (&window_light->gui, window_light->gc, 0, 0, LIGHT_WIDTH, LIGHT_HEIGHT);
	
	gc_set_fc (window_light->gc, COLOR_LIGHT_FOREGROUND);	
	gc_set_bc (window_light->gc, COLOR_LIGHT_BACKGROUND);
	gc_set_lt (window_light->gc, 1);
	
	if (selected_channel != NULL)
	{
		font.size = 24;
		gc_set_fc (window_light->gc, COLOR_LIGHT_CHANNEL_FOREGROUND);	
		font_draw_str (&font, selected_channel->name, strlen (selected_channel->name), &window_light->gui, window_light->gc, 0, 4);
		//gc_set_fc (window_light->gc, COLOR_LIGHT_FOREGROUND);	
		
		if (selected_title != NULL)
		{
			light_update_show_title (selected_title, 0);
			
			gc_set_fc (window_light->gc, COLOR_LIGHT_CHANNEL_FOREGROUND);	
			char *description = epgdb_read_long_description (selected_title);
			textarea (window_light, description, 600, 48, LIGHT_WIDTH - 600, LIGHT_HEIGHT - 60, 16, 18, 3, 0);
			_free (description);
			//gc_set_fc (window_light->gc, COLOR_LIGHT_FOREGROUND);	
			
			if (selected_title->next != NULL)
			{
				light_update_show_title (selected_title->next, 1);
			}
		}
	}
	
	if (selected_group != NULL)
	{
		int width;
		font.size = 24;
		gc_set_fc (window_light->gc, COLOR_LIGHT_CHANNEL_FOREGROUND);	
		gc_set_bc (window_light->gc, COLOR_LIGHT_BACKGROUND);
		width = font_width_str (&font, selected_group->name, strlen (selected_group->name));
		font_draw_str (&font, selected_group->name, strlen (selected_group->name), &window_light->gui, window_light->gc, LIGHT_WIDTH - width, 4);
	}

	wm_redraw (window_light);
}

static void light_show ()
{
	if (window_light) light_update ();
	else
	{
		rect_t rect = { LIGHT_X_OFFSET, LIGHT_Y_OFFSET, LIGHT_WIDTH, LIGHT_HEIGHT };
		window_light = wm_create (&rect, "", NULL, NULL, COLOR_TRANSPARENT);
		light_update ();
		wm_show (window_light);	
	}
}

static void light_hide ()
{
	if (window_light) wm_destroy (window_light);
	window_light = NULL;
}
