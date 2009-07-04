static int remote_yellow_event_handler (event_t *event)
{	
	if (event_key_2value (event) == event_key_value_up) return 0;
	
	int key = event_key_2code (event);
	//log_add("%x", key);
	
	if (key == KEY_UP)
		title_prev ();
	
	if (key == KEY_DOWN)
		title_next ();
		
	if (key == KEY_LEFT)
		channels_prev ();
	
	if (key == KEY_RIGHT)
		channels_next ();
	
	if (key == 0x40)		// circular arrows
		title_link ();
	
	if (key == 0x68)		// page up
		group_next ();
	
	if (key == 0x6d)		// page down
		group_prev ();
	
	if (key == 0x160)		// ok button
		title_ok (true);
	
	if (key == 0xa7)		// rec button
		title_rec ();
	
	if (key == 0x8b)		// menu button
	{
		int menu_ret = show_menu ();
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
		title_show_info ();
	
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
