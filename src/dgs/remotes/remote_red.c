static int remote_red_event_handler (event_t *event)
{	
	if (event_key_2value (event) == event_key_value_up) return 0;

	int key = event_key_2code (event);
	
	if (key == KEY_UP)
		channels_prev ();
	
	if (key == KEY_DOWN)
		channels_next ();
		
	if (key == KEY_LEFT)
		title_prev ();
	
	if (key == KEY_RIGHT)
		title_next ();
	
	if (key == 0x40)		// circular arrows
		title_link ();
	
	if (key == 0x68)		// page up
		group_next ();
	
	if (key == 0x6d)		// page down
		group_prev ();
	
	if (key == 0x160)		// ok button
		title_ok ();
	
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
		channels_prev_page ();
	
	if (key == 0xbc)		// vol down
		channels_next_page ();
		
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
