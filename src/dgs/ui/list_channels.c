static window_t *window_list_channels = NULL;

#define LIST_CHANNELS_X_OFFSET	22
#define LIST_CHANNELS_Y_OFFSET	290
#define LIST_CHANNELS_WIDTH		300
#define LIST_CHANNELS_HEIGHT	210

static void list_channels_init ()
{
	rect_t rect = { LIST_CHANNELS_X_OFFSET, LIST_CHANNELS_Y_OFFSET, LIST_CHANNELS_WIDTH, LIST_CHANNELS_HEIGHT };
	window_list_channels = wm_create (&rect, "", NULL, NULL, COLOR_TRANSPARENT);
}

static void list_channels_show ()
{
	wm_show (window_list_channels);	
}

static void list_channels_clean ()
{
	wm_destroy (window_list_channels);
}

static void list_channels_update ()
{
	struct _font font;
	time_t now = time (NULL) - (dgs_helper_get_daylight_saving () * (60 * 60));
	
	gc_set_fc (window_list_channels->gc, COLOR_LIST_CHANNELS_BACKGROUND);
	gt_fillrect (&window_list_channels->fb, window_list_channels->gc, 0, 0, LIST_CHANNELS_WIDTH, LIST_CHANNELS_HEIGHT);

	gc_set_fc (window_list_channels->gc, COLOR_LIST_CHANNELS_FOREGROUND);	
	gc_set_bc (window_list_channels->gc, COLOR_LIST_CHANNELS_BACKGROUND);

	if (start_channel != NULL)
	{
		int i;
		dgs_channel_t *tmp = start_channel;
		for (i=0; (i<8) && (i<dgs_channels_count ()); i++)
		{
			int offset;
			epgdb_title_t *title;
			char *title_text = NULL;
			int perc;
			
			if (tmp == selected_channel)
			{
				if (selected_column == 0)
				{
					gc_set_fc (window_list_channels->gc, COLOR_LIST_CHANNELS_SELECTED_BACKGROUND);
					gt_fillrect (&window_list_channels->fb, window_list_channels->gc, 0, (26 * i), LIST_CHANNELS_WIDTH, 26);
					gc_set_bc (window_list_channels->gc, COLOR_LIST_CHANNELS_SELECTED_BACKGROUND);
					gc_set_fc (window_list_channels->gc, COLOR_LIST_CHANNELS_SELECTED_FOREGROUND);
				}
				else
				{
					gc_set_fc (window_list_channels->gc, COLOR_LIST_CHANNELS_SELECTED_BACKGROUND);
					gc_set_bc (window_list_channels->gc, COLOR_LIST_CHANNELS_SELECTED_FOREGROUND);
					gc_set_lt (window_list_channels->gc, 2);
					gt_rect (&window_list_channels->fb, window_list_channels->gc, 1, (26 * i)+1, LIST_CHANNELS_WIDTH-2, 24);
					gc_set_bc (window_list_channels->gc, COLOR_LIST_CHANNELS_BACKGROUND);
					gc_set_fc (window_list_channels->gc, COLOR_LIST_CHANNELS_FOREGROUND);
				}
			}
			else
			{
				gc_set_bc (window_list_channels->gc, COLOR_LIST_CHANNELS_BACKGROUND);
				gc_set_fc (window_list_channels->gc, COLOR_LIST_CHANNELS_FOREGROUND);
			}
			
			if (tmp->db_channel != NULL)
			{
			    title = epgdb_titles_get_by_time (tmp->db_channel, now);
				if (title != NULL)
				{
					if ((title->start_time <= now) && ((title->start_time + title->length) >= now))
					{
						title_text = epgdb_read_description (title);
						perc = ((now - title->start_time) * 100) / title->length;
					}
				}
			}
			
			font.size = 15;
			offset = font_width_str (&font, tmp->name, strlen (tmp->name));
			font_draw_str (&font, tmp->name, strlen (tmp->name), &window_list_channels->fb, window_list_channels->gc, 5, (i*26)+4);
			
			if (title_text != NULL)
			{
				char row[1024];
				char *text = ui_resize_string (title_text, font.size, LIST_CHANNELS_WIDTH - offset - 60);
				sprintf (row, "(%s - %d%%)", text, perc);
				font.size = 14;
				font_draw_str (&font, row, strlen (row), &window_list_channels->fb, window_list_channels->gc, offset + 15, (i*26)+4);
				_free (text);
				_free (title_text);
			}
			
			tmp = dgs_channels_get_next (tmp);
		}
	}
	
	wm_redraw (window_list_channels);
}
