void show_config ()
{
	int i, j;
	w_config_t mywindow;
	w_config_item_t items[6];
	
	/* check for log default path */
	int db_def_path = -1;
	if (strcmp (config_get_db_root (), "/media") == 0)
	{
		db_def_path = 0;
	}
	else if (strcmp (config_get_db_root (), "/mnt/usb") == 0)
	{
		db_def_path = 1;
	}

	/* create db values */
	int db_count = 0;
	w_config_value_t *db_values;
	if (db_def_path == -1)
	{
		db_values = _malloc (sizeof (w_config_value_t) * 3);
		db_values[0].name = intl (CUSTOM);
		db_count++;
	}
	else db_values = _malloc (sizeof (w_config_value_t) * 2);
	db_values[db_count].name = intl (HARD_DISK);
	db_count++;
	db_values[db_count].name = intl (USB_PEN);
	db_count++;
	items[0].type = 0;
	items[0].name = intl (DATABASE_PATH);
	items[0].values = db_values;
	items[0].values_count = db_count;
	if (db_def_path == -1)
		items[0].selected_value = 0;
	else
		items[0].selected_value = db_def_path;
	items[0].changed = false;
	
	/* check for log default path */
	int log_def_path = -1;
	if (strcmp (config_get_log_file (), "/media/crossepg.log") == 0)
	{
		if (strcmp (config_get_dwnl_log_file (), "/media/crossepg.downloader.log") == 0)
			if (strcmp (config_get_sync_log_file (), "/media/crossepg.sync.log") == 0)
				if (strcmp (config_get_launcher_log_file (), "/media/crossepg.launcher.log") == 0)
					log_def_path = 0;
	}
	else if (strcmp (config_get_log_file (), "/mnt/usb/crossepg.log") == 0)
	{
		if (strcmp (config_get_dwnl_log_file (), "/mnt/usb/crossepg.downloader.log") == 0)
			if (strcmp (config_get_sync_log_file (), "/mnt/usb/crossepg.sync.log") == 0)
				if (strcmp (config_get_launcher_log_file (), "/mnt/usb/crossepg.launcher.log") == 0)
					log_def_path = 1;
	}
	else if (strcmp (config_get_log_file (), "/tmp/crossepg.log") == 0)
	{
		if (strcmp (config_get_dwnl_log_file (), "/tmp/crossepg.downloader.log") == 0)
			if (strcmp (config_get_sync_log_file (), "/tmp/crossepg.sync.log") == 0)
				if (strcmp (config_get_launcher_log_file (), "/tmp/crossepg.launcher.log") == 0)
					log_def_path = 2;
	}
	
	/* create log values */
	int log_count = 0;
	w_config_value_t *log_values;
	if (log_def_path == -1)
	{
		log_values = _malloc (sizeof (w_config_value_t) * 4);
		log_values[0].name = intl (CUSTOM);
		log_count++;
	}
	else log_values = _malloc (sizeof (w_config_value_t) * 3);
	log_values[log_count].name = intl (HARD_DISK);
	log_count++;
	log_values[log_count].name = intl (USB_PEN);
	log_count++;
	log_values[log_count].name = intl (TEMP_FOLDER);
	log_count++;
	items[1].type = 0;
	items[1].name = intl (LOGS_PATH);
	items[1].values = log_values;
	items[1].values_count = log_count;
	if (log_def_path == -1)
		items[1].selected_value = 0;
	else
		items[1].selected_value = log_def_path;
	items[1].changed = false;

	/* sync hours */
	items[2].type = 1;
	items[2].name = intl (SYNC_HOURS);
	items[2].selected_value = config_get_sync_hours ();
	items[2].min = 0;
	items[2].max = 240;
	items[2].step = 6;
	items[2].changed = false;
	
	/* read groups */
	int *syncgroups = config_get_sync_groups ();
	w_config_value_t *groups = _malloc (dgs_groups_count () * sizeof (w_config_value_t));
	dgs_group_t *group = dgs_groups_get_first ();
	int count = 0;
	while (group != NULL)
	{
		groups[count].id = group->id;
		groups[count].name = group->name;
		groups[count].selected = false;
		for (i=0; i<64; i++)
			if (syncgroups[i] == group->id)
			{
				groups[count].selected = true;
				break;
			}
			
		group = group->next;
		count++;
	}
	
	/* sync groups */
	items[3].type = 2;
	items[3].name = intl (SYNC_GROUPS);
	items[3].values = groups;
	items[3].values_count = count;
	items[3].selected_value = 0;
	items[3].changed = false;	

	/* opentv provider */
	
	int provider_count = 0;
	w_config_value_t provider_values[64];
	DIR *dd = opendir ("/var/crossepg/providers");
	struct dirent *file;
	
	items[4].selected_value = 0;
	if (dd != NULL)
	{
		while ((file = readdir (dd)) != NULL)
		{
			int length = strlen (file->d_name);
			if (length > 5)
			{
				if (strcmp (file->d_name + length - 5, ".conf"))
				{
					provider_values[provider_count].name = _malloc (length - 4);
					memcpy (provider_values[provider_count].name, file->d_name, length - 5);
					provider_values[provider_count].name[length-5] = '\0';
					provider_values[provider_count].selected = false;
					for (i=0; i<64; i++)
					{
						if (strcmp (provider_values[provider_count].name, config_get_otv_provider (i)) == 0)
						{
							provider_values[provider_count].selected = true;
							break;
						}
					}
					provider_count++;
				}
			}
		}
		closedir (dd);
	}
	
	items[4].type = 2;
	items[4].name = intl (OPENTV_PROVIDER);
	items[4].values = provider_values;
	items[4].values_count = provider_count;
	items[4].selected_value = 0;
	items[4].changed = false;
	
	/* channel pause */
	items[5].type = 1;
	items[5].name = "Change ch. sleep (sec)";
	items[5].selected_value = config_get_channel_sleep ();
	items[5].min = 0;
	items[5].max = 99;
	items[5].step = 1;
	items[5].changed = false;
	
	/* draw config window */
	mywindow.title = "System";
	mywindow.items = items;
	mywindow.items_count = 6;
	mywindow.selected_item = 0;
	mywindow.start_item = 0;
	window_config_init (&mywindow);
	window_config_update (&mywindow);
	window_config_show (&mywindow);
	window_config_execute (&mywindow);
	window_config_clean (&mywindow);
	
	if (mywindow.exit_ok)
	{
		if ((db_def_path != -1) || (mywindow.items[0].selected_value != 0))
		{
			if (db_def_path == -1)
				mywindow.items[0].selected_value--;
				
			switch (mywindow.items[0].selected_value)
			{
				case 0:
					config_set_db_root ("/media");
					break;
				case 1:
					config_set_db_root ("/mnt/usb");
					break;
			}
		}
		
		if ((log_def_path != -1) || (mywindow.items[1].selected_value != 0))
		{
			if (log_def_path == -1)
				mywindow.items[1].selected_value--;
				
			switch (mywindow.items[1].selected_value)
			{
				case 0:
					config_set_log_file ("/media/crossepg.log");
					config_set_dwnl_log_file ("/media/crossepg.downloader.log");
					config_set_sync_log_file ("/media/crossepg.sync.log");
					config_set_launcher_log_file ("/media/crossepg.launcher.log");
					break;
				case 1:
					config_set_log_file ("/mnt/usb/crossepg.log");
					config_set_dwnl_log_file ("/mnt/usb/crossepg.downloader.log");
					config_set_sync_log_file ("/mnt/usb/crossepg.sync.log");
					config_set_launcher_log_file ("/mnt/usb/crossepg.launcher.log");
					break;
				case 2:
					config_set_log_file ("/tmp/crossepg.log");
					config_set_dwnl_log_file ("/tmp/crossepg.downloader.log");
					config_set_sync_log_file ("/tmp/crossepg.sync.log");
					config_set_launcher_log_file ("/tmp/crossepg.launcher.log");
					break;
			}
		}
		
		config_set_sync_hours (mywindow.items[2].selected_value);
		
		for (i=0; i<64; i++)
			config_set_sync_groups (i, 0);
			
		j=0;
		for (i=0; i<items[3].values_count; i++)
		{
			if (items[3].values[i].selected)
			{
				config_set_sync_groups (j, items[3].values[i].id);
				j++;
			}
		}

		j=0;
		for (i=0; i<items[4].values_count; i++)
		{
			if (items[4].values[i].selected)
			{
				config_set_otv_provider (j, items[4].values[i].name);
				j++;
			}
		}
		for (i=j; i<64; i++)
			config_set_otv_provider (i, "\0");
		
		config_set_channel_sleep (mywindow.items[5].selected_value);
		
		if (config_save ()) log_add ("Configuration saved");
		else log_add ("Error saving configuration");
	}
	
	for (i=0; i<items[4].values_count; i++)
		_free (items[4].values[i].name);
	
	_free (db_values);
	_free (log_values);
	_free (groups);
}

void show_aspect_config ()
{
	int i;
	w_config_t mywindow;
	w_config_item_t items[3];
	
	/* skin */
	int skins_count = 0;
	w_config_value_t skins_values[64];
	DIR *dd = opendir ("/var/crossepg/skins");
	struct dirent *file;
	
	items[0].selected_value = 0;
	if (dd != NULL)
	{
		while ((file = readdir (dd)) != NULL)
		{
			int length = strlen (file->d_name);
			if (length > 3)
			{
				skins_values[skins_count].name = _malloc (length+1);
				strcpy (skins_values[skins_count].name, file->d_name);
				if (strlen (file->d_name) == (strlen (config_get_skin ())))
					if (memcmp (file->d_name, config_get_skin (), strlen (file->d_name)) == 0)
						items[0].selected_value = skins_count;
				
				skins_count++;
			}
		}
		closedir (dd);
	}
	
	items[0].type = 0;
	items[0].name = intl (SKIN);
	items[0].values = skins_values;
	items[0].values_count = skins_count;
	items[0].changed = false;
	
	/* start screen */
	w_config_value_t start_screen_values[4];
	
	start_screen_values[0].name = intl (GRID);
	start_screen_values[1].name = intl (LIST);
	start_screen_values[2].name = intl (LIGHT);
	start_screen_values[3].name = intl (SCHEDULER);
	items[1].selected_value = config_get_start_screen ();
	items[1].type = 0;
	items[1].name = "Start screen";
	items[1].values = start_screen_values;
	items[1].values_count = 4;
	items[1].changed = false;

	/* read groups */

	int *visiblegroups = config_get_visible_groups ();
	w_config_value_t *groups = _malloc (dgs_groups_count () * sizeof (w_config_value_t));
	dgs_group_t *group = dgs_groups_get_first ();
	int count = 0;
	while (group != NULL)
	{
		groups[count].id = group->id;
		groups[count].name = group->name;
		groups[count].selected = false;
		for (i=0; i<64; i++)
			if (visiblegroups[i] == group->id)
			{
				groups[count].selected = true;
				break;
			}
			
		group = group->next;
		count++;
	}

	/* visible groups */

	items[2].type = 2;
	items[2].name = "Visible groups";
	items[2].values = groups;
	items[2].values_count = count;
	items[2].selected_value = 0;
	items[2].changed = false;	

	/* draw config window */
	mywindow.title = "Aspect";
	mywindow.items = items;
	mywindow.items_count = 3;
	mywindow.selected_item = 0;
	mywindow.start_item = 0;
	window_config_init (&mywindow);
	window_config_update (&mywindow);
	window_config_show (&mywindow);
	window_config_execute (&mywindow);
	window_config_clean (&mywindow);
	
	if (mywindow.exit_ok)
	{
		int j;
		
		config_set_skin (mywindow.items[0].values[mywindow.items[0].selected_value].name);
		config_set_start_screen (mywindow.items[1].selected_value);
		
		for (i=0; i<64; i++)
			config_set_visible_groups (i, 0);
			
		j=0;
		for (i=0; i<items[2].values_count; i++)
		{
			if (items[2].values[i].selected)
			{
				config_set_visible_groups (j, items[2].values[i].id);
				j++;
			}
		}
		
		if (config_save ()) log_add ("Configuration saved");
		else log_add ("Error saving configuration");
		
		images_clean ();
		images_load ();
		colors_read ();
	}
	
	for (i=0; i<items[0].values_count; i++)
		_free (items[0].values[i].name);
	
	_free (groups);
}

void show_actions_config ()
{
	w_config_t mywindow;
	w_config_item_t items[3];
	
	/* boot action */
	w_config_value_t boot_action_values[4];
	
	boot_action_values[0].name = "No action";
	boot_action_values[1].name = "Download";
	boot_action_values[2].name = "Sync";
	boot_action_values[3].name = "Download & Sync";
	items[0].selected_value = config_get_boot_action ();
	items[0].type = 0;
	items[0].name = "Boot action";
	items[0].values = boot_action_values;
	items[0].values_count = 4;
	items[0].changed = false;
	
	/* daily action */
	w_config_value_t daily_action_values[4];
	daily_action_values[0].name = "No action";
	daily_action_values[1].name = "Download";
	daily_action_values[2].name = "Sync";
	daily_action_values[3].name = "Download & Sync";
	items[1].selected_value = config_get_cron_action ();
	items[1].type = 0;
	items[1].name = "Daily action";
	items[1].values = daily_action_values;
	items[1].values_count = 4;
	items[1].changed = false;
	
	/* daily hour */
	items[2].type = 1;
	items[2].name = "Hour of daily action";
	items[2].selected_value = config_get_cron_hour ();
	items[2].min = 0;
	items[2].max = 23;
	items[2].step = 1;
	items[2].changed = false;
	
	/* draw config window */
	mywindow.title = "Automatic actions";
	mywindow.items = items;
	mywindow.items_count = 3;
	mywindow.selected_item = 0;
	mywindow.start_item = 0;
	window_config_init (&mywindow);
	window_config_update (&mywindow);
	window_config_show (&mywindow);
	window_config_execute (&mywindow);
	window_config_clean (&mywindow);
	
	if (mywindow.exit_ok)
	{
		config_set_boot_action (mywindow.items[0].selected_value);
		config_set_cron_action (mywindow.items[1].selected_value);
		config_set_cron_hour (mywindow.items[2].selected_value);
		
		if (config_save ()) log_add ("Configuration saved");
		else log_add ("Error saving configuration");
	}
}

int show_menu ()
{
	w_menu_t mywindow;
	char *items[5];
	items[0] = intl (START_SYNC);
	items[1] = intl (START_DOWNLOADER);
	items[2] = "System";
	items[3] = "Aspect";
	items[4] = "Automatic actions";
	mywindow.title = intl (MENU);
	mywindow.items = items;
	mywindow.items_count = 5;
	mywindow.selected_item = 0;
	window_menu_init (&mywindow);
	window_menu_update (&mywindow);
	window_menu_show (&mywindow);
	
	while (true)
	{
		window_menu_execute (&mywindow);

		if (mywindow.exit_ok)
		{
			switch (mywindow.selected_item)
			{
				case 0:
					window_menu_clean (&mywindow);
					log_add ("Launching SYNC");
					return -2;
					
				case 1:
					window_menu_clean (&mywindow);
					log_add ("Launching DOWNLOADER");
					return -3;
					
				case 2:
					show_config ();
					break;
					
				case 3:
					show_aspect_config ();
					break;
					
				case 4:
					show_actions_config ();
					break;
			}
		}
		else break;
	}
	
	window_menu_clean (&mywindow);
	return 0;
}

static void channels_prev ()
{
	if (selected_channel == NULL) return;
	if (start_channel == NULL) return;
	
	if (selected_channel == start_channel)
	{
		selected_channel = dgs_channels_get_prev (selected_channel);
		start_channel = selected_channel;
	}
	else
		selected_channel = dgs_channels_get_prev (selected_channel);

	if (selected_channel != NULL)
	    selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, selected_time - (dgs_helper_get_daylight_saving () * (60 * 60)));
	else
		selected_title = NULL;

	start_title = selected_title;

	if (selected_title != NULL)
	{
		if ((dgs_helper_adjust_daylight (selected_title->start_time) + selected_title->length) <= start_time)
			start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (selected_title->start_time % (60*60));

		if (dgs_helper_adjust_daylight (selected_title->start_time) >= (start_time + GRID_X_TIME))
			start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (selected_title->start_time % (60*60));
	}
}

static void channels_next ()
{
	if (selected_channel == NULL) return;
    if (start_channel == NULL) return;
		
	int i;
	dgs_channel_t *tmp;
	if (selected_channel == NULL) return;
	
	tmp = start_channel;
	for (i=1; i<8; i++) tmp = dgs_channels_get_next (tmp);
	
	if (selected_channel == tmp)
	{
		selected_channel = dgs_channels_get_next (selected_channel);
		start_channel = dgs_channels_get_next (start_channel);
	}
	else
		selected_channel = dgs_channels_get_next (selected_channel);
	
	if (selected_channel != NULL)
	    selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, selected_time - (dgs_helper_get_daylight_saving () * (60 * 60)));
	else
		selected_title = NULL;
	
	start_title = selected_title;
	
	if (selected_title != NULL)
	{
		if ((dgs_helper_adjust_daylight (selected_title->start_time) + selected_title->length) <= start_time)
			start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (selected_title->start_time % (60*60));

		if (dgs_helper_adjust_daylight (selected_title->start_time) >= (start_time + GRID_X_TIME))
			start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (selected_title->start_time % (60*60));
	}
}

static void group_prev ()
{
	int i, j;
	bool withfilter = false;
	dgs_group_t *tmp;
	int *groupsfilter = config_get_visible_groups ();
	for (i=0; i<64; i++)
		if (groupsfilter[i] != 0)
		{
			withfilter = true;
			break;
		}
		
	if (!withfilter) tmp = dgs_groups_get_prev (selected_group);
	else
	{
		bool ended = false;
		tmp = selected_group;
		dgs_group_t *tmp2 = selected_group;
		for (i=0; (i < dgs_groups_count ()) && (tmp2 != NULL); i++)
		{
			tmp2 = dgs_groups_get_prev (tmp2);
			for (j=0; j<64; j++)
			{
				if (groupsfilter[j] == tmp2->id)
				{
					tmp = tmp2;
					ended = true;
					break;
				}
			}
			if (ended) break;
		}
	}
	
	if ((tmp != NULL) && (tmp != selected_group))
	{
		dgs_channels_clean ();
		dgs_channels_init ();
		selected_group = tmp;
		dgs_channels_readgroup (selected_group->id);
		dgs_channels_link_to_db ();
		selected_channel = dgs_channels_get_first ();
		start_channel = selected_channel;
		if (selected_channel != NULL)
		    selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, selected_time - (dgs_helper_get_daylight_saving () * (60 * 60)));
		else
			selected_title = NULL;
			
		start_title = selected_title;
	}
}

static void group_next ()
{
	int i, j;
	bool withfilter = false;
	dgs_group_t *tmp;
	int *groupsfilter = config_get_visible_groups ();
	for (i=0; i<64; i++)
		if (groupsfilter[i] != 0)
		{
			withfilter = true;
			break;
		}
		
	if (!withfilter) tmp = dgs_groups_get_next (selected_group);
	else
	{
		bool ended = false;
		tmp = selected_group;
		dgs_group_t *tmp2 = selected_group;
		for (i=0; (i < dgs_groups_count ()) && (tmp2 != NULL); i++)
		{
			tmp2 = dgs_groups_get_next (tmp2);
			for (j=0; j<64; j++)
			{
				if (groupsfilter[j] == tmp2->id)
				{
					tmp = tmp2;
					ended = true;
					break;
				}
			}
			if (ended) break;
		}
	}
	
	if ((tmp != NULL) && (tmp != selected_group))
	{
		dgs_channels_clean ();
		dgs_channels_init ();
		selected_group = tmp;
		dgs_channels_readgroup (selected_group->id);
		dgs_channels_link_to_db ();
		selected_channel = dgs_channels_get_first ();
		start_channel = selected_channel;
		if (selected_channel != NULL)
		    selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, selected_time - (dgs_helper_get_daylight_saving () * (60 * 60)));
		else
			selected_title = NULL;
			
		start_title = selected_title;
	}
}

static void title_prev ()
{
	if (selected_channel == NULL) return;
	if (selected_channel->db_channel == NULL) return;
	if (selected_title == NULL) return;
	
	if (selected_title == start_title)
	{
		if (selected_title->prev != NULL) selected_title = selected_title->prev;
		if (start_title->prev != NULL) start_title = start_title->prev;
	}
	else
		if (selected_title->prev != NULL) selected_title = selected_title->prev;
		
	if ((dgs_helper_adjust_daylight (selected_title->start_time) + selected_title->length) <= start_time)
		start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*60));

	selected_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*30)) + (60*30);
}

static void title_next ()
{
	if (selected_channel == NULL) return;
	if (selected_channel->db_channel == NULL) return;
	if (selected_title == NULL) return;
	
	int i;
	epgdb_title_t *tmp = start_title;
	for (i=1; (i<8) && (tmp!=NULL); i++) tmp = tmp->next;

	if (selected_title == tmp)
	{
		if (selected_title->next != NULL) selected_title = selected_title->next;
		if (start_title->next != NULL) start_title = start_title->next;
	}
	else
		if (selected_title->next != NULL) selected_title = selected_title->next;
		
	if (dgs_helper_adjust_daylight (selected_title->start_time) >= (start_time + GRID_X_TIME))
		start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*60));

	selected_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*30)) + (60*30);
}

static void title_now ()
{
	time_t now = time (NULL);
	if (selected_channel == NULL) return;
	if (selected_channel->db_channel == NULL) return;
	if (selected_title == NULL) return;
	selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, now - (dgs_helper_get_daylight_saving () * (60 * 60)));
	start_title = selected_title;

	if (dgs_helper_adjust_daylight (selected_title->start_time) >= (start_time + GRID_X_TIME))
		start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*60));
		
	if ((dgs_helper_adjust_daylight (selected_title->start_time) + selected_title->length) <= start_time)
		start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*60));

	selected_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*30)) + (60*30);
}

static void title_prev_24h ()
{
	if (selected_channel == NULL) return;
	if (selected_channel->db_channel == NULL) return;
	if (selected_title == NULL) return;
	selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, selected_title->start_time - (60*60*24) - (dgs_helper_get_daylight_saving () * (60 * 60)));
	start_title = selected_title;

	if ((dgs_helper_adjust_daylight (selected_title->start_time) + selected_title->length) <= start_time)
		start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*60));

	selected_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*30)) + (60*30);
}

static void title_next_24h ()
{
	if (selected_channel == NULL) return;
	if (selected_channel->db_channel == NULL) return;
	if (selected_title == NULL) return;
	selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, selected_title->start_time + (60*60*24) - (dgs_helper_get_daylight_saving () * (60 * 60)));
	start_title = selected_title;

	if (dgs_helper_adjust_daylight (selected_title->start_time) >= (start_time + GRID_X_TIME))
		start_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*60));

	selected_time = dgs_helper_adjust_daylight (selected_title->start_time) - (dgs_helper_adjust_daylight (selected_title->start_time) % (60*30)) + (60*30);
}

static void title_rec ()
{
	scheduler_t *scheduler;
	bool add = true;
	
	if (selected_channel == NULL) return;
	if (selected_title == NULL) return;

	scheduler = scheduler_get_by_channel_and_title (selected_channel, selected_title);
	
	window_progress_set_pos (240, 200);
	window_progress_set_show_bar (false);
	window_progress_init ();
	window_progress_update ("", "", 0);
	window_progress_show ();
	
	if (scheduler != NULL)
	{
		if (scheduler->type == 0) add = false;
		if ((scheduler->type == 0) || (scheduler->type == 1))
		{
			window_progress_update (intl (SCHEDULER_DEL), "", 0);
			scheduler_del (selected_channel->id, dgs_helper_adjust_daylight (selected_title->start_time), selected_title->length, scheduler->type, true);
		}
	}
	
	if (add)
	{
		bool added;
		window_progress_update (intl (SCHEDULER_ADD), "", 0);
		char *description = epgdb_read_description (selected_title);
		added = scheduler_add (selected_channel->id, dgs_helper_adjust_daylight (selected_title->start_time), selected_title->length, 0, description, true);
		_free (description);
		// TODO: intl ()
		if (!added) show_message_box (intl (ERROR), "Cannot add event. No free tuner", 0);
	}
	window_progress_clean ();
}

static void title_link ()
{
	if (selected_channel == NULL) return;
	if (selected_title == NULL) return;

	char *description = epgdb_read_description (selected_title);
	scheduler_add (selected_channel->id, dgs_helper_adjust_daylight (selected_title->start_time), selected_title->length, 2, description, true);
	_free (description);
	
	window_progress_set_pos (240, 200);
	window_progress_set_show_bar (false);
	window_progress_init ();
	window_progress_update (intl (LINKING_EVENTS), "", 0);
	window_progress_show ();
	scheduler_link ();
	window_progress_clean ();
}

static int title_ok ()
{
	time_t tmp_time = time (NULL);
	if (selected_channel == NULL) return 0;
	if ((selected_title == NULL) || (dgs_helper_adjust_daylight (selected_title->start_time) <= tmp_time))
	{
		int ch_id = ch_watching_id (ch_mode_live);
		if (ch_id == selected_channel->id) return -1;
		ch_change_fg (ch_mode_live, selected_channel->id, NULL);
		return 0;
	}
	
	bool add = true;
	scheduler_t *scheduler = scheduler_get_by_channel_and_title (selected_channel, selected_title);

	window_progress_set_pos (240, 200);
	window_progress_set_show_bar (false);
	window_progress_init ();
	window_progress_update ("", "", 0);
	window_progress_show ();

	if (scheduler != NULL)
	{
		if (scheduler->type == 1) add = false;
		if ((scheduler->type == 0) || (scheduler->type == 1))
		{
			window_progress_update (intl (SCHEDULER_DEL), "", 0);
			scheduler_del (selected_channel->id, dgs_helper_adjust_daylight (selected_title->start_time), selected_title->length, scheduler->type, true);
		}
	}

	if (add)
	{
		bool added;
		window_progress_update (intl (SCHEDULER_ADD), "", 0);
		char *description = epgdb_read_description (selected_title);
		added = scheduler_add (selected_channel->id, dgs_helper_adjust_daylight (selected_title->start_time), selected_title->length, 1, description, true);
		_free (description);
		// TODO: intl ()
		if (!added) show_message_box (intl (ERROR), "Cannot add event. No free tuner", 0);
	}
	window_progress_clean ();
	
	return 0;
}

static void title_show_info ()
{
	if (selected_title == NULL) return;
	if (selected_channel == NULL) return;
	//if (!selected_extra_info) return;
	
	char *description = epgdb_read_long_description (selected_title);
	char *name = epgdb_read_description (selected_title);
	
	epgdb_search_res_t *similar = epgdb_search_by_name (name, 32);
	if (similar->count > 0)
	{
		int nid = 0, tsid = 0, sid = 0;
		char *channel_name = "";
		char similar_text[64*1024];
		char *tmp = similar_text;
		sprintf (tmp, "\n\nSimilar events\n");
		tmp += strlen (tmp);
		int i;
		
		for (i=0; i<similar->count; i++)
		{
			if ((similar->channels[i]->nid != selected_channel->nid) || (similar->channels[i]->tsid != selected_channel->tsid) || (similar->channels[i]->sid != selected_channel->sid) || (similar->titles[i]->start_time != selected_title->start_time))
			{
				if ((similar->channels[i]->nid != nid) || (similar->channels[i]->tsid != tsid) || (similar->channels[i]->sid != sid))
				{
					channel_name = dgs_helper_get_channel_name (similar->channels[i]->nid, similar->channels[i]->tsid, similar->channels[i]->sid);
					nid = similar->channels[i]->nid;
					tsid = similar->channels[i]->tsid;
					sid = similar->channels[i]->sid;
				}
				if (strlen (channel_name) > 0)
				{
					time_t event_time = dgs_helper_adjust_daylight (similar->titles[i]->start_time);
					struct tm *loctime;
					loctime = localtime (&event_time);
					strftime (tmp, 255, "%d/%m/%Y %H:%M:%S", loctime);
					tmp += strlen (tmp);
					sprintf (tmp, " - %s\n", channel_name);
					tmp += strlen (tmp);
				}
			}
		}
		if (similar->count == 32)
			sprintf (tmp, "and others ...\n");
		
		tmp = description;
		description = _malloc (strlen (tmp) + strlen (similar_text) + 1);
		memset (description, 0, strlen (tmp) + strlen (similar_text) + 1);
		memcpy (description, tmp, strlen (tmp));
		_free (tmp);
		tmp = description;
		tmp += strlen (tmp);
		memcpy (tmp, similar_text, strlen (similar_text));
	}
	
	show_message_box_with_pos (name, description, 2, 50, 80, 620, 420, "info_large.bmp");
	
	epgdb_search_free (similar);
	_free (name);
	_free (description);
}

static void channels_prev_page ()
{
	if (selected_channel == NULL) return;
    if (start_channel == NULL) return;

	int i;
	for (i=0; i<8; i++)
	{
		selected_channel = dgs_channels_get_prev (selected_channel);
		start_channel = dgs_channels_get_prev (start_channel);
	}
	
	if (selected_channel != NULL)
		selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, selected_time - (dgs_helper_get_daylight_saving () * (60 * 60)));
	else
		selected_title = NULL;

	start_title = selected_title;

	if (selected_title != NULL)
	{
		if ((selected_title->start_time + selected_title->length) <= start_time)
			start_time = selected_title->start_time - (selected_title->start_time % (60*60));

		if (selected_title->start_time >= (start_time + GRID_X_TIME))
			start_time = selected_title->start_time - (selected_title->start_time % (60*60));
	}
}

static void channels_next_page ()
{
	if (selected_channel == NULL) return;
    if (start_channel == NULL) return;
		
	int i;
	for (i=0; i<8; i++)
	{
		selected_channel = dgs_channels_get_next (selected_channel);
		start_channel = dgs_channels_get_next (start_channel);
	}	

    if (selected_channel != NULL)
	    selected_title = epgdb_titles_get_by_time (selected_channel->db_channel, selected_time - (dgs_helper_get_daylight_saving () * (60 * 60)));
	else
        selected_title = NULL;
	
	start_title = selected_title;
	
	if (selected_title != NULL)
	{
		if ((selected_title->start_time + selected_title->length) <= start_time)
			start_time = selected_title->start_time - (selected_title->start_time % (60*60));

		if (selected_title->start_time >= (start_time + GRID_X_TIME))
			start_time = selected_title->start_time - (selected_title->start_time % (60*60));
	}
}
