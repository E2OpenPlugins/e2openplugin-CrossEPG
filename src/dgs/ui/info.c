static window_t *window_info = NULL;

#define INFO_X_OFFSET 	20
#define INFO_Y_OFFSET 	50
#define INFO_WIDTH		430
#define INFO_HEIGHT		210

static void info_update ()
{
	struct _font font;
	int group_size = 0;
	//int daylight = dgs_helper_get_daylight_saving () * (60 * 60);
	gc_set_fc (window_info->gc, COLOR_INFO_BACKGROUND);
	gt_fillrect (&window_info->fb, window_info->gc, 0, 0, INFO_WIDTH, INFO_HEIGHT);

	gc_set_fc (window_info->gc, COLOR_INFO_FOREGROUND);	
	gc_set_bc (window_info->gc, COLOR_INFO_BACKGROUND);
	gc_set_lt (window_info->gc, 1);
	gt_rect (&window_info->fb, window_info->gc, 0, 30, INFO_WIDTH, 1);

	if (selected_group != NULL)
	{
		font.size = 17;
		gc_set_fc (window_info->gc, COLOR_INFO_FOREGROUND);	
		gc_set_bc (window_info->gc, COLOR_INFO_BACKGROUND);
		group_size = font_width_str (&font, selected_group->name, strlen (selected_group->name));
		font_draw_str (&font, selected_group->name, strlen (selected_group->name), &window_info->fb, window_info->gc, INFO_WIDTH - group_size - 4, 2);
	}

	if (selected_title != NULL)
	{
		scheduler_t *scheduler;
		char *description = epgdb_read_long_description (selected_title);
		char *name = epgdb_read_description (selected_title);
		struct tm loctime;
		struct tm loctime_now;
		char start_time[256];
		char message[256];
		char missing[256];
		time_t now;
		
		font.size = 17;
		char *resized_name = ui_resize_string (name, font.size, INFO_WIDTH - group_size - 14);
		font_draw_str (&font, resized_name, strlen (resized_name), &window_info->fb, window_info->gc, 4, 2);
		_free (resized_name);
		_free (name);
		
		textarea (window_info, description, 4, 34, INFO_WIDTH - 8, INFO_HEIGHT - 30, 16, 20, 6, 0);
		
		_free (description);
		
		now = time (NULL);	
		if ((dgs_helper_adjust_daylight (selected_title->start_time) + selected_title->length) < now)
		{
			// elapsed
			sprintf (missing, intl (ELAPSED));
		}
		else if (dgs_helper_adjust_daylight (selected_title->start_time) > now)
		{
			// in the future
			int minutes = (dgs_helper_adjust_daylight (selected_title->start_time) - now) / 60;
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
			sprintf (missing, intl (STARTED), (int)(now - dgs_helper_adjust_daylight (selected_title->start_time)) / 60);
			if (selected_title->length > 0)
			{
				int perc_width;
				perc_width = (INFO_WIDTH * (now - dgs_helper_adjust_daylight (selected_title->start_time))) / selected_title->length;
				gt_fillrect (&window_info->fb, window_info->gc, 0, 26, perc_width, 4);
			}
		}
		time_t tmptime = dgs_helper_adjust_daylight (selected_title->start_time);
		localtime_r (&tmptime, &loctime);
		strftime (start_time, 255, intl (TIME_HM), &loctime);
		
		localtime_r (&now, &loctime_now);
		if ((loctime.tm_year == loctime_now.tm_year) && (loctime.tm_mon == loctime_now.tm_mon) && (loctime.tm_mday == loctime_now.tm_mday))
			sprintf (message, intl (START_TODAY), start_time, (selected_title->length / 60));
		else
			sprintf (message, intl (START), intl (SUNDAY + loctime.tm_wday), loctime.tm_mday, intl (JANUARY + loctime.tm_mon), start_time, (selected_title->length / 60));
		
		font.size = 14;
		font_draw_str (&font, missing, strlen (missing), &window_info->fb, window_info->gc, 4, INFO_HEIGHT - 40);
		font_draw_str (&font, message, strlen (message), &window_info->fb, window_info->gc, 4, INFO_HEIGHT - 22);
		
		scheduler = scheduler_get_by_channel_and_title (selected_channel, selected_title);
		if (scheduler != NULL)
		{
			font.size = 14;
			if (scheduler->type == 0)
			{
				if (bullet_red)
					img_draw (&window_info->fb, bullet_red, window_info->gc, INFO_WIDTH - 16, INFO_HEIGHT - 20, 14, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			}
			else if (scheduler->type == 1)
			{
				if (bullet_blue)
					img_draw (&window_info->fb, bullet_blue, window_info->gc, INFO_WIDTH - 16, INFO_HEIGHT - 20, 14, 14, IMG_ALIGN_CENTER|IMG_ALIGN_MIDDLE);
			}
		}
	}

	wm_redraw (window_info);
}

static void info_show ()
{
	if (window_info) info_update ();
	else
	{
		rect_t rect = { INFO_X_OFFSET, INFO_Y_OFFSET, INFO_WIDTH, INFO_HEIGHT };
		window_info = wm_create (&rect, "INFO", NULL, NULL, COLOR_TRANSPARENT);
		info_update ();
		wm_show (window_info);	
	}
}

static void info_hide ()
{
	if (window_info) wm_destroy (window_info);
	window_info = NULL;
}
