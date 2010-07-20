static window_t *window_list_scheduler = NULL;

#define LIST_SCHEDULER_X_OFFSET	35
#define LIST_SCHEDULER_Y_OFFSET	120
#define LIST_SCHEDULER_WIDTH	1195
#define LIST_SCHEDULER_HEIGHT	470

static void list_scheduler_init ()
{
	rect_t rect = { LIST_SCHEDULER_X_OFFSET, LIST_SCHEDULER_Y_OFFSET, LIST_SCHEDULER_WIDTH, LIST_SCHEDULER_HEIGHT };
	window_list_scheduler = wm_create (&rect, "", NULL, NULL, COLOR_TRANSPARENT);
}

static void list_scheduler_show ()
{
	wm_show (window_list_scheduler);	
}

static void list_scheduler_clean ()
{
	wm_destroy (window_list_scheduler);
}

static void list_scheduler_update ()
{
	struct _font font;
	int i = 0;
	
	gc_set_fc (window_list_scheduler->gc, COLOR_LIST_SCHEDULER_BACKGROUND);
	gt_fillrect (&window_list_scheduler->gui, window_list_scheduler->gc, 0, 0, LIST_SCHEDULER_WIDTH, LIST_SCHEDULER_HEIGHT);
	
	scheduler_t *sch = start_scheduler;
	
	while (sch != NULL)
	{
		struct tm loctime;
		char time_string[10];
		char date_string[20];
		char length_string[64];
		char *channel_name = dgs_helper_get_channel_name_by_id (sch->channel_id);
		font.size = 21;
		
		if (sch == selected_scheduler)
		{
			gc_set_fc (window_list_scheduler->gc, COLOR_LIST_SCHEDULER_SELECTED_BACKGROUND);
			gt_fillrect (&window_list_scheduler->gui, window_list_scheduler->gc, 0, (29 * i), LIST_SCHEDULER_WIDTH, 29);
			gc_set_fc (window_list_scheduler->gc, COLOR_LIST_SCHEDULER_SELECTED_FOREGROUND);
			gc_set_bc (window_list_scheduler->gc, COLOR_LIST_SCHEDULER_SELECTED_BACKGROUND);
		}
		else
		{
			gc_set_fc (window_list_scheduler->gc, COLOR_LIST_SCHEDULER_FOREGROUND);	
			gc_set_bc (window_list_scheduler->gc, COLOR_LIST_SCHEDULER_BACKGROUND);
		}
		
		font_draw_str (&font, channel_name, strlen (channel_name), &window_list_scheduler->gui, window_list_scheduler->gc, 5, (i*29)+4);
		
		if (sch->name != NULL)
			font_draw_str (&font, sch->name, strlen (sch->name), &window_list_scheduler->gui, window_list_scheduler->gc, 140, (i*29)+4);
		else
			font_draw_str (&font, intl (NO_TITLE), strlen (intl (NO_TITLE)), &window_list_scheduler->gui, window_list_scheduler->gc, 5, (i*29)+4);
		
		if (sch->type == 0)
			font_draw_str (&font, intl (RECORD), strlen (intl (RECORD)), &window_list_scheduler->gui, window_list_scheduler->gc, 750, (i*29)+4);
		else if (sch->type == 1)
			font_draw_str (&font, intl (ZAP), strlen (intl (ZAP)), &window_list_scheduler->gui, window_list_scheduler->gc, 750, (i*29)+4);
		else if (sch->type == 2)
			font_draw_str (&font, intl (LINKED), strlen (intl (LINKED)), &window_list_scheduler->gui, window_list_scheduler->gc, 750, (i*29)+4);
		
		localtime_r (&sch->start_time, &loctime);
		strftime (date_string, 20, intl (DATE_DM), &loctime);
		if (sch->type < 2)
			font_draw_str (&font, date_string, strlen (date_string), &window_list_scheduler->gui, window_list_scheduler->gc, 960, (i*29)+4);
		strftime (time_string, 10, intl (TIME_HM), &loctime);
		font_draw_str (&font, time_string, strlen (time_string), &window_list_scheduler->gui, window_list_scheduler->gc, 1030, (i*29)+4);
		sprintf (length_string, "%d min", (sch->length / 60));
		font_draw_str (&font, length_string, strlen (length_string), &window_list_scheduler->gui, window_list_scheduler->gc, 1100, (i*29)+4);
		sch = sch->next;
		i++;
		if (i==8) break;
	}
	wm_redraw (window_list_scheduler);
}
