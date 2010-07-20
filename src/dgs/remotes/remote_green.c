static int remote_green_event_handler (event_t *event)
{	
	if (event_key_2value (event) == event_key_value_up) return 0;
	
	int key = event_key_2code (event);
	
	if (key == KEY_UP)
	{
		if (selected_column == 0)
		{
			channels_prev ();
			title_now ();
			return 1;
		}
		else
		{
			title_prev ();
			return 2;
		}
	}
	
	if (key == KEY_DOWN)
	{
		if (selected_column == 0)
		{
			channels_next ();
			title_now ();
			return 1;
		}
		else
		{
			title_next ();
			return 2;
		}
	}
	
	if ((key == KEY_LEFT) || (key == KEY_RIGHT))
	{
		if (selected_column == 0) selected_column = 1;
		else selected_column = 0;
	}
	
	if (key == 0x40)		// circular arrows
		title_link ();
		
	if (key == 0x68)		// page up
		group_next ();
	
	if (key == 0x6d)		// page down
		group_prev ();
	
	if (key == 0x160)		// ok button
	{
		if (title_ok (false) == -1)
		{
			selected_screen = -1;
			return -1;
		}
	}
	
	if (key == 0xa7)		// rec button
		title_rec ();
	
	if (key == 0x8b)		// menu button
	{
		plug_event_close ();
		int menu_ret = show_menu ();
		plug_event_init ();
		if (menu_ret < 0)
		{
			selected_screen = -1;
			return menu_ret;
		}
	}
	
	if (key == 0xcf)		// play
		title_now ();
	
	if (key == 0xd0)		// fwd
		title_next_24h ();
	
	if (key == 0xa8)		// rew
		title_prev_24h ();
	
	if (key == 0x166)		// i
	{
		plug_event_close ();
		title_show_info ();
		plug_event_init ();
	}
	
	if (key == 0xbb)		// vol up
	{
		if (selected_column == 0)
		{
			channels_prev_page ();
			title_now ();
		}
		else
		{
			int i;
			for (i=0;i<10;i++) title_prev ();
		}
	}
	
	if (key == 0xbc)		// vol down
	{
		if (selected_column == 0)
		{
			channels_next_page ();
			title_now ();
		}
		else
		{
			int i;
			for (i=0;i<10;i++) title_next ();
		}
	}
	
	if (key == 0x18e)		// red button
	{
		selected_screen = 0;
		return -1;
	}
	
	if (key == 0x190)		// yellow button
	{
		selected_screen = 2;
		return -1;
	}
	
	if (key == 0x191)		// blue button
	{
		selected_screen = 3;
		return -1;
	}
	
	if (key == KEY_BACK)
	{
		selected_screen = -1;
		return -1;
	}
	
	return 0;
}
