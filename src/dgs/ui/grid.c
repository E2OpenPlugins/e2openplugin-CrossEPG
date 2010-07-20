static window_t *window_grid = NULL;

#define GRID_X_OFFSET 	35
#define GRID_Y_OFFSET 	270
#define GRID_WIDTH		1195
#define GRID_HEIGHT		(((GRID_ROWS+1)*GRID_ROW_HEIGHT)+2)
#define GRID_ROW_HEIGHT 29
#define GRID_ROWS 		10

#define GRID_X_TIME    (3*60*60)
#define GRID_SPP       (GRID_X_TIME / GRID_EVENT_WIDTH)

#define GRID_CHANNEL_WIDTH	200
#define GRID_EVENT_WIDTH    (GRID_WIDTH-GRID_CHANNEL_WIDTH)

static void grid_init ()
{
	rect_t rect = { GRID_X_OFFSET, GRID_Y_OFFSET, GRID_WIDTH, GRID_HEIGHT };
	window_grid = wm_create (&rect, "GRID", NULL, NULL, COLOR_TRANSPARENT);
}

static void grid_show ()
{
	wm_show (window_grid);	
}

static void grid_clean ()
{
	wm_destroy (window_grid);
}

static char *ui_resize_string (char *text, int font_size, int width)
{
	int size;
	int ipotetic_font_size;
	int ipotetic_chars;
	struct _font font;
	bool dirback = false;
	char *ret;

	font.size = font_size;
	
	ipotetic_font_size = (font_size * 60) / 100;
	ipotetic_chars = width / ipotetic_font_size;
	if (ipotetic_chars > strlen (text)) ipotetic_chars = strlen (text);
	if (ipotetic_chars < 1) ipotetic_chars = 0;
	
	while (true)
	{
		size = font_width_str (&font, text, ipotetic_chars);

		if (size < width)
		{
			int forward;
			
			if (dirback) break;
			if (ipotetic_chars == strlen (text)) break;
			if ((size + ipotetic_font_size) > width) break;
			
			forward = (width - size) / ipotetic_font_size;
			if (forward <= 0) forward = 1;
			ipotetic_chars += forward;
			if (ipotetic_chars > strlen (text)) ipotetic_chars = strlen (text);
		}
		else
		{
			int back = (size - width) / ipotetic_font_size;
			if (back == 0) back = 1;
			ipotetic_chars -= back;
			if (ipotetic_chars < 1)
			{
				ipotetic_chars = 0;
				break;
			}
			dirback = true;
		}
	}

	if (ipotetic_chars == strlen (text))
	{
		ret = _malloc (ipotetic_chars + 1);
		memcpy (ret, text, ipotetic_chars);
		ret[ipotetic_chars] = '\0';
	}
	else if (ipotetic_chars < 2)
	{
		ret = _malloc (1);
		ret[0] = '\0';
	}
	else
	{
		ret = _malloc (ipotetic_chars + 2);
		memcpy (ret, text, ipotetic_chars - 2);
		ret[ipotetic_chars-2] = '.';
		ret[ipotetic_chars-1] = '.';
		ret[ipotetic_chars] = '.';
		ret[ipotetic_chars+1] = '\0';
	}
	
	return ret;
}

static int grid_drawbox (dgs_channel_t *channel, epgdb_title_t *title, int row)
{
	int x, width, offset;
	char *text, *fulltext;
	struct _font font;
	int sch_mode = -1;
	scheduler_t *scheduler;
	//int daylight = dgs_helper_get_daylight_saving () * (60 * 60);

	if ((dgs_helper_adjust_daylight (title->start_time) + title->length) <= start_time) return -1;
	if (dgs_helper_adjust_daylight (title->start_time) >= (start_time + GRID_X_TIME)) return 1;
	font.size = 16;
	
	/* calculate x and width */

	x = ((dgs_helper_adjust_daylight (title->start_time) - start_time) / GRID_SPP);
	if (x < 0)
	{
		offset = x;
		x = 0;
	}
	else
		offset = 0;
	
	width = (title->length / GRID_SPP) + offset;
	if ((x + width) > GRID_EVENT_WIDTH) width = GRID_EVENT_WIDTH - x;
	x += GRID_CHANNEL_WIDTH;
	
	/* check if scheduled */
	
	scheduler = scheduler_get_by_channel_and_title (channel, title);
	if (scheduler != NULL) sch_mode = scheduler->type;
	
	/* adjust the string length */

	fulltext = epgdb_read_description (title);
	text = ui_resize_string (fulltext, font.size, width-10);
	_free (fulltext);

	if (selected_title == title)
	{
		gc_set_fc (window_grid->gc, COLOR_GRID_TITLES_SELECTED_BACKGROUND);
		gt_fillrect (&window_grid->gui, window_grid->gc, x + 1, ((row + 1) * GRID_ROW_HEIGHT) + 2, width - 1, GRID_ROW_HEIGHT - 3);
		gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
		gc_set_bc (window_grid->gc, COLOR_GRID_TITLES_SELECTED_BACKGROUND);
		gt_rect (&window_grid->gui, window_grid->gc, x, ((row + 1) * GRID_ROW_HEIGHT) + 1, width, GRID_ROW_HEIGHT - 1);
		gc_set_fc (window_grid->gc, COLOR_GRID_TITLES_SELECTED_FOREGROUND);
		font_draw_str (&font, text, strlen (text), &window_grid->gui, window_grid->gc, x + 5, ((row + 1) * GRID_ROW_HEIGHT) + 4);
	}
	else if (sch_mode == 0)
	{
		gc_set_fc (window_grid->gc, COLOR_GRID_TITLES_REC_BACKGROUND);
		gt_fillrect (&window_grid->gui, window_grid->gc, x + 1, ((row + 1) * GRID_ROW_HEIGHT) + 2, width - 1, GRID_ROW_HEIGHT - 3);
		gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
		gc_set_bc (window_grid->gc, COLOR_GRID_TITLES_REC_BACKGROUND);
		gt_rect (&window_grid->gui, window_grid->gc, x, ((row + 1) * GRID_ROW_HEIGHT) + 1, width, GRID_ROW_HEIGHT - 1);
		gc_set_fc (window_grid->gc, COLOR_GRID_TITLES_REC_FOREGROUND);
		font_draw_str (&font, text, strlen (text), &window_grid->gui, window_grid->gc, x + 5, ((row + 1) * GRID_ROW_HEIGHT) + 4);
	}
	else if (sch_mode == 1)
	{
		gc_set_fc (window_grid->gc, COLOR_GRID_TITLES_ZAP_BACKGROUND);
		gt_fillrect (&window_grid->gui, window_grid->gc, x + 1, ((row + 1) * GRID_ROW_HEIGHT) + 2, width - 1, GRID_ROW_HEIGHT - 3);
		gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
		gc_set_bc (window_grid->gc, COLOR_GRID_TITLES_ZAP_BACKGROUND);
		gt_rect (&window_grid->gui, window_grid->gc, x, ((row + 1) * GRID_ROW_HEIGHT) + 1, width, GRID_ROW_HEIGHT - 1);
		gc_set_fc (window_grid->gc, COLOR_GRID_TITLES_ZAP_FOREGROUND);
		font_draw_str (&font, text, strlen (text), &window_grid->gui, window_grid->gc, x + 5, ((row + 1) * GRID_ROW_HEIGHT) + 4);
	}
	else
	{		
		gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
		gc_set_bc (window_grid->gc, COLOR_GRID_TITLES_BACKGROUND);
		gt_rect (&window_grid->gui, window_grid->gc, x, ((row + 1) * GRID_ROW_HEIGHT) + 1, 1, GRID_ROW_HEIGHT - 1);
		gt_rect (&window_grid->gui, window_grid->gc, x+width, ((row + 1) * GRID_ROW_HEIGHT) + 1, 1, GRID_ROW_HEIGHT - 1);
		gc_set_fc (window_grid->gc, COLOR_GRID_TITLES_FOREGROUND);
		font_draw_str (&font, text, strlen (text), &window_grid->gui, window_grid->gc, x + 5, ((row + 1) * GRID_ROW_HEIGHT) + 4);
	}
	
	_free (text);
	return 0;
}

static void grid_update ()
{
	struct _font font;
	struct tm *loctime;
	time_t sel_time;
	time_t now_time = time (NULL);
	char string[256];
	int i;
	dgs_channel_t *tmp;
	
	/* draw background */
	gc_set_fc (window_grid->gc, COLOR_TRANSPARENT);
	gt_fillrect (&window_grid->gui, window_grid->gc, 0, 0, GRID_WIDTH, GRID_HEIGHT);

	gc_set_fc (window_grid->gc, COLOR_GRID_HEADERS_BACKGROUND);
	gt_fillrect (&window_grid->gui, window_grid->gc, 0, 0, GRID_WIDTH, GRID_ROW_HEIGHT);
	
	gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
	gc_set_bc (window_grid->gc, COLOR_GRID_HEADERS_BACKGROUND);
	gc_set_lt (window_grid->gc, 1);
	gt_rect (&window_grid->gui, window_grid->gc, 1, 1, GRID_CHANNEL_WIDTH - 1, GRID_ROW_HEIGHT - 1);
	gt_rect (&window_grid->gui, window_grid->gc, GRID_CHANNEL_WIDTH, 1, GRID_EVENT_WIDTH - 2, GRID_ROW_HEIGHT - 1);
	
	for (i=1; i<=GRID_ROWS; i++)
	{
		gc_set_fc (window_grid->gc, COLOR_GRID_CHANNELS_BACKGROUND);
		gt_fillrect (&window_grid->gui, window_grid->gc, 1, GRID_ROW_HEIGHT * i, GRID_CHANNEL_WIDTH, GRID_ROW_HEIGHT - 1);

		gc_set_fc (window_grid->gc, COLOR_GRID_TITLES_BACKGROUND);
		gt_fillrect (&window_grid->gui, window_grid->gc, GRID_CHANNEL_WIDTH, GRID_ROW_HEIGHT * i, GRID_EVENT_WIDTH - 2, GRID_ROW_HEIGHT - 1);

		gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
		gc_set_bc (window_grid->gc, COLOR_GRID_CHANNELS_BACKGROUND);
		gt_rect (&window_grid->gui, window_grid->gc, 1, GRID_ROW_HEIGHT * i, GRID_CHANNEL_WIDTH, GRID_ROW_HEIGHT - 1);

		gc_set_bc (window_grid->gc, COLOR_GRID_TITLES_BACKGROUND);
		gt_rect (&window_grid->gui, window_grid->gc, GRID_CHANNEL_WIDTH, GRID_ROW_HEIGHT * i, GRID_EVENT_WIDTH - 2, GRID_ROW_HEIGHT - 1);
	}
	
	/* draw now marker */

	if ((now_time > start_time) && (now_time < (start_time + GRID_X_TIME)))
	{
		int x;
		x = ((now_time - start_time) / GRID_SPP);
		x += GRID_CHANNEL_WIDTH;
		gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
		gc_set_lt (window_grid->gc, 2);
		gt_rect (&window_grid->gui, window_grid->gc, x, GRID_ROW_HEIGHT - 8, 0, 6);
		gc_set_lt (window_grid->gc, 1);
	}
	
	/* draw header */

	font.size = 18;
	sel_time = start_time;
	loctime = localtime (&sel_time);
	strftime (string, 20, intl (DATE_DM), loctime);
	gc_set_fc (window_grid->gc, COLOR_GRID_HEADERS_FOREGROUND);
	gc_set_bc (window_grid->gc, COLOR_GRID_HEADERS_BACKGROUND);
	font_draw_str (&font, string, strlen(string), &window_grid->gui, window_grid->gc, 2, 2);
		
	font.size = 14;
	strftime (string, 10, intl (TIME_HM), loctime);
	font_draw_str (&font, string, strlen(string), &window_grid->gui, window_grid->gc, GRID_CHANNEL_WIDTH + 2, 2);

	sel_time = sel_time + (60*60);
	loctime = localtime (&sel_time);
	strftime (string, 10, intl (TIME_HM), loctime);
	font_draw_str (&font, string, strlen(string), &window_grid->gui, window_grid->gc, GRID_CHANNEL_WIDTH + (GRID_EVENT_WIDTH * 0.33) + 2, 2);
	gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
	gt_rect (&window_grid->gui, window_grid->gc, GRID_CHANNEL_WIDTH + (GRID_EVENT_WIDTH * 0.33), 1, 0, GRID_ROW_HEIGHT - 2);
	gc_set_fc (window_grid->gc, COLOR_GRID_HEADERS_FOREGROUND);

	sel_time = sel_time + (60*60);
	loctime = localtime (&sel_time);
	strftime (string, 10, intl (TIME_HM), loctime);
	font_draw_str (&font, string, strlen(string), &window_grid->gui, window_grid->gc, GRID_CHANNEL_WIDTH + (GRID_EVENT_WIDTH * 0.66) + 2, 2);
	gc_set_fc (window_grid->gc, COLOR_GRID_BORDER);
	gt_rect (&window_grid->gui, window_grid->gc, GRID_CHANNEL_WIDTH + (GRID_EVENT_WIDTH * 0.66), 1, 0, GRID_ROW_HEIGHT - 2);
	gc_set_fc (window_grid->gc, COLOR_GRID_HEADERS_FOREGROUND);

	sel_time = sel_time + (60*60);
	loctime = localtime (&sel_time);
	strftime (string, 10, intl (TIME_HM), loctime);
	font_draw_str (&font, string, strlen(string), &window_grid->gui, window_grid->gc, GRID_WIDTH - 42, 2);

	/* draw elements */

	if (start_channel != NULL)
	{
		tmp = start_channel;
		for (i=0; (i<10) && (i<dgs_channels_count ()); i++)
		{
			epgdb_title_t *title;
			char *text;
			font.size = 18;
			if (tmp == selected_channel)
			{
				gc_set_fc (window_grid->gc, COLOR_GRID_CHANNELS_SELECTED_BACKGROUND);
				gt_fillrect (&window_grid->gui, window_grid->gc, 2, (GRID_ROW_HEIGHT * (i+1)) + 1, GRID_CHANNEL_WIDTH - 2, GRID_ROW_HEIGHT - 2);
				gc_set_bc (window_grid->gc, COLOR_GRID_CHANNELS_SELECTED_BACKGROUND);
				gc_set_fc (window_grid->gc, COLOR_GRID_CHANNELS_SELECTED_FOREGROUND);
			}
			else
			{
				gc_set_bc (window_grid->gc, COLOR_GRID_CHANNELS_BACKGROUND);
				gc_set_fc (window_grid->gc, COLOR_GRID_CHANNELS_FOREGROUND);
			}
			
			text = ui_resize_string (tmp->name, font.size, GRID_CHANNEL_WIDTH - 5);
			font_draw_str (&font, text, strlen (text), &window_grid->gui, window_grid->gc, 5, ((i+1)*GRID_ROW_HEIGHT)+3);
			_free (text);
			
			if (tmp->db_channel != NULL)
			{
				title = tmp->db_channel->title_first;
				while (title != NULL)
				{
					if (grid_drawbox (tmp, title, i) == 1) break;
					title = title->next;
				}
			}
						
			tmp = dgs_channels_get_next (tmp);
		}
	}
		
	wm_redraw (window_grid);
}
