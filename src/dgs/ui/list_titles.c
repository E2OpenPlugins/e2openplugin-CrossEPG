static window_t *window_list_titles = NULL;
// 274

#define LIST_TITLES_X_OFFSET	620
#define LIST_TITLES_Y_OFFSET	290
#define LIST_TITLES_WIDTH		580
#define LIST_TITLES_HEIGHT		300

static void list_titles_init ()
{
	rect_t rect = { LIST_TITLES_X_OFFSET, LIST_TITLES_Y_OFFSET, LIST_TITLES_WIDTH, LIST_TITLES_HEIGHT };
	window_list_titles = wm_create (&rect, "", NULL, NULL, COLOR_TRANSPARENT);
}

static void list_titles_show ()
{
	wm_show (window_list_titles);	
}

static void list_titles_clean ()
{
	wm_destroy (window_list_titles);
}

static void list_titles_update ()
{
	struct _font font;
	time_t now = time (NULL) - (dgs_helper_get_daylight_saving () * (60 * 60));
	
	gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_BACKGROUND);
	gt_fillrect (&window_list_titles->gui, window_list_titles->gc, 0, 0, LIST_TITLES_WIDTH, LIST_TITLES_HEIGHT);

	gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_FOREGROUND);	
	gc_set_bc (window_list_titles->gc, COLOR_LIST_TITLES_BACKGROUND);
	
	if (start_title != NULL)
	{
		epgdb_title_t *title = start_title;
		int i;
		for (i=0; (i<10) && (title != NULL); i++)
		{
			int sch_mode = -1;
			scheduler_t *scheduler = scheduler_get_by_channel_and_title (selected_channel, title);
			if (scheduler != NULL) sch_mode = scheduler->type;
			
			if (title == selected_title)
			{
				if (selected_column == 1)
				{
					gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_SELECTED_BACKGROUND);
					gt_fillrect (&window_list_titles->gui, window_list_titles->gc, 0, (29 * i), LIST_TITLES_WIDTH, 29);
					gc_set_bc (window_list_titles->gc, COLOR_LIST_TITLES_SELECTED_BACKGROUND);
					if (sch_mode == 0) gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_REC_FOREGROUND);
					else if (sch_mode == 1) gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_ZAP_FOREGROUND);
					else gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_SELECTED_FOREGROUND);
				}
				else
				{
					gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_SELECTED_BACKGROUND);
					gc_set_bc (window_list_titles->gc, COLOR_LIST_TITLES_SELECTED_FOREGROUND);
					gc_set_lt (window_list_titles->gc, 2);
					gt_rect (&window_list_titles->gui, window_list_titles->gc, 1, (29 * i)+1, LIST_TITLES_WIDTH-2, 27);
					gc_set_bc (window_list_titles->gc, COLOR_LIST_TITLES_BACKGROUND);
					if (sch_mode == 0) gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_REC_FOREGROUND);
					else if (sch_mode == 1) gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_ZAP_FOREGROUND);
					else gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_FOREGROUND);
				}
			}
			else if (sch_mode == 0)
			{
				gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_REC_BACKGROUND);
				gt_fillrect (&window_list_titles->gui, window_list_titles->gc, 0, (29 * i), LIST_TITLES_WIDTH, 29);
				gc_set_bc (window_list_titles->gc, COLOR_LIST_TITLES_REC_BACKGROUND);
				gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_REC_FOREGROUND);
			}
			else if (sch_mode == 1)
			{
				gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_ZAP_BACKGROUND);
				gt_fillrect (&window_list_titles->gui, window_list_titles->gc, 0, (29 * i), LIST_TITLES_WIDTH, 29);
				gc_set_bc (window_list_titles->gc, COLOR_LIST_TITLES_ZAP_BACKGROUND);
				gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_ZAP_FOREGROUND);
			}
			else
			{
				gc_set_bc (window_list_titles->gc, COLOR_LIST_TITLES_BACKGROUND);
				gc_set_fc (window_list_titles->gc, COLOR_LIST_TITLES_FOREGROUND);
			}
			struct tm loctime, loctime_now;
			char time_string[10];
			char *title_text = epgdb_read_description (title);
			time_t start_time = dgs_helper_adjust_daylight (title->start_time);
			localtime_r (&start_time, &loctime);
			localtime_r (&now, &loctime_now);
			
			font.size = 18;
			int offset;
			
			if ((loctime.tm_year == loctime_now.tm_year) && (loctime.tm_mon == loctime_now.tm_mon) && (loctime.tm_mday == loctime_now.tm_mday))
			{
				offset = font_width_str (&font, intl (TODAY), strlen (intl (TODAY)));
				font_draw_str (&font, intl (TODAY), strlen (intl (TODAY)), &window_list_titles->gui, window_list_titles->gc, 5, (i*29)+4);
			}
			else
			{
				offset = font_width_str (&font, intl (SUNDAY + loctime.tm_wday), strlen (intl (SUNDAY + loctime.tm_wday)));
				font_draw_str (&font, intl (SUNDAY + loctime.tm_wday), strlen (intl (SUNDAY + loctime.tm_wday)), &window_list_titles->gui, window_list_titles->gc, 5, (i*29)+4);
			}
			
			strftime (time_string, 10, intl (TIME_HM), &loctime);
			font_draw_str (&font, time_string, strlen (time_string), &window_list_titles->gui, window_list_titles->gc, offset + 12, (i*29)+4);
			char *text = ui_resize_string (title_text, font.size, LIST_TITLES_WIDTH - offset - 60);
			font_draw_str (&font, text, strlen (text), &window_list_titles->gui, window_list_titles->gc, offset + 70, (i*29)+4);
			_free (title_text);
			_free (text);
			title = title->next;
		}
	}
	wm_redraw (window_list_titles);
}
