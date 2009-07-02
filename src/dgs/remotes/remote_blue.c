static int remote_blue_event_handler (event_t *event)
{	
	if (event_key_2value (event) == event_key_value_up) return 0;
	
	int key = event_key_2code (event);
	//log_add("%x", key);
	
	if (key == KEY_UP)
	{
		if (selected_scheduler->prev != NULL)
		{
			if (start_scheduler == selected_scheduler) start_scheduler = start_scheduler->prev;
			selected_scheduler = selected_scheduler->prev;
		}
	}
	
	if (key == KEY_DOWN)
	{
		if (selected_scheduler->next != NULL)
		{
			int i;
			scheduler_t *tmp = start_scheduler;
			for (i=0; (i<7)&&(tmp!=NULL); i++) tmp = tmp->next;
			if (tmp == selected_scheduler) start_scheduler = start_scheduler->next;
			selected_scheduler = selected_scheduler->next;
		}
	}

	if (key == 0x80)		// stop button
	{
		if (selected_scheduler != NULL)
		{
			window_progress_set_pos (240, 200);
			window_progress_set_show_bar (false);
			window_progress_init ();
			window_progress_update (intl (SCHEDULER_DEL), "", 0);
			window_progress_show ();
			scheduler_del (selected_scheduler->channel_id, selected_scheduler->start_time, selected_scheduler->length, selected_scheduler->type, true);
			window_progress_clean ();
			selected_scheduler = scheduler_get_first ();
			start_scheduler = scheduler_get_first ();
		}
	}
	
	if (key == 0x40)		// circular arrows
	{
		window_progress_set_pos (240, 200);
		window_progress_set_show_bar (false);
		window_progress_init ();
		window_progress_update (intl (LINKING_EVENTS), "", 0);
		window_progress_show ();
		scheduler_link ();
		sleep (1);
		window_progress_clean ();
		selected_scheduler = scheduler_get_first ();
		start_scheduler = scheduler_get_first ();
	}
	
	if (key == 0x8b)		// menu button
	{
		int menu_ret = show_menu ();
		if (menu_ret < 0)
		{
			selected_screen = -1;
			return menu_ret;
		}
	}
	
	if (key == 0x18e)		// red button
	{
		selected_screen = 0;
		return -1;
	}
	
	if (key == 0x18f)		// green button
	{
		selected_screen = 1;
		return -1;
	}
	
	if (key == 0x190)		// yellow button
	{
		selected_screen = 2;
		return -1;
	}
	
	if (key == KEY_BACK)
	{
		selected_screen = -1;
		return -1;
	}
	
	return 0;
}
