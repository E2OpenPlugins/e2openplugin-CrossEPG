int _aliases_auto_callback_a (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
	int i;
	alias_t *aliases = (alias_t*)p_data;
	
	for (i=0; i<aliases_count; i++)
	{
		if ((atoi (p_fields[1]) == aliases[i].nid[0]) && (atoi (p_fields[2]) == aliases[i].tsid[0]) && (atoi (p_fields[3]) == aliases[i].sid[0]))
		{
			if (aliases[i].name != NULL) _free (aliases[i].name);
			aliases[i].name = _malloc (strlen (p_fields[0])+1);
			strcpy (aliases[i].name, p_fields[0]);
		}
	}
	return 0;
}

bool _alias_exist (alias_t *aliases, int nid, int tsid, int sid)
{
	int i, j;
	for (i=0; i<aliases_count; i++)
	{
		for (j=0; j<aliases[i].count; j++)
		{
			if ((nid == aliases[i].nid[j]) && (tsid == aliases[i].tsid[j]) && (sid == aliases[i].sid[j])) return true;
		}
	}
	return false;
}

int _alias_get (alias_t *aliases, int nid, int tsid, int sid)
{
	int i, j;
	for (i=0; i<aliases_count; i++)
	{
		for (j=0; j<aliases[i].count; j++)
		{
			if ((nid == aliases[i].nid[j]) && (tsid == aliases[i].tsid[j]) && (sid == aliases[i].sid[j])) return i;
		}
	}
	return -1;
}

int _aliases_auto_callback_b (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
	int i, j;
	alias_t *aliases = (alias_t*)p_data;
	int name_length = strlen (p_fields[0]);
	
	for (i=0; i<aliases_count; i++)
	{
		if (aliases[i].name != NULL)
		{
			if (strlen (aliases[i].name) == name_length)
			{
				char a[name_length];
				char b[name_length];
				for (j=0; j<name_length; j++)
				{
					a[j] = towupper (aliases[i].name[j]);
					b[j] = towupper (p_fields[0][j]);
				}
				
				if (memcmp (a, b, name_length) == 0)
				{
					int nid = atoi (p_fields[1]);
					int tsid = atoi (p_fields[2]);
					int sid = atoi (p_fields[3]);
					
					if (!_alias_exist (aliases, nid, tsid, sid) && (aliases[i].count < 32))
					{
						aliases[i].nid[aliases[i].count] = nid;
						aliases[i].tsid[aliases[i].count] = tsid;
						aliases[i].sid[aliases[i].count] = sid;
						aliases[i].count++;
						log_add ("Add alias on channel '%s' (nid: %d tsid: %d sid: %d)", aliases[i].name, nid, tsid, sid);
					}
				}
			}
		}
	}
	return 0;
}

void aliases_auto (int group, char *filename)
{
	int res, i;
	char *errmsg = 0;
	aliases_count = epgdb_channels_count ();
	alias_t aliases[aliases_count];
	char sql[1024];

	epgdb_channel_t *channel = epgdb_channels_get_first ();
	int counter = 0;
	while (channel != NULL)
	{
		aliases[counter].name = NULL;
		aliases[counter].nid[0] = channel->nid;
		aliases[counter].tsid[0] = channel->tsid;
		aliases[counter].sid[0] = channel->sid;
		aliases[counter].count = 1;
		
		counter++;
		channel = channel->next;
	}
	
	if (dgs_getdb () == NULL) return;
	
	log_add ("Reading channel names...");
	res = sqlite3_exec (dgs_getdb (), "SELECT fullname, org_network_id, ts_id, service_id FROM channelinfo", _aliases_auto_callback_a, &aliases, &errmsg);
	
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);

	log_add ("Searching for similar...");
	if (group < 0)
		sprintf (sql, "SELECT fullname, org_network_id, ts_id, service_id FROM channelinfo");
	else
		sprintf (sql, "SELECT channelinfo.fullname, channelinfo.org_network_id, channelinfo.ts_id, channelinfo.service_id FROM channelinfo INNER JOIN FAVListInfo ON channelinfo.id = FAVListInfo.ch_id WHERE FAVListInfo.grp_id = '%d'", group);
	res = sqlite3_exec (dgs_getdb (), sql, _aliases_auto_callback_b, &aliases, &errmsg);
	
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
		
	if (!_aliases_auto_save (aliases, filename))
		log_add ("Cannot save aliases on file '%s'", filename);
	
	for (i=0; i<aliases_count; i++)
		if (aliases[i].name != NULL)
			_free (aliases[i].name);

	return;
}

void aliases_add (char *channel_a, char *channel_b, char *filename)
{
	alias_t aliases[1024];
	_aliases_load (aliases, 1024, filename);
	int nid_a[32], tsid_a[32], sid_a[32], count_a;
	int nid_b[32], tsid_b[32], sid_b[32], count_b;
	int i;
	int channel_a_length = strlen (channel_a);
	int channel_b_length = strlen (channel_b);
	int group_a = -1;
	int group_b = -1;
	FILE *fd;
	
	char *tmp = strtok (channel_a, ":");
	if (tmp != NULL)
	{
		if (strlen (tmp) < channel_a_length)
		{
			group_a = atoi (tmp);
			channel_a = strtok (NULL, "\0");
		}
	}
	
	tmp = strtok (channel_b, ":");
	if (tmp != NULL)
	{
		if (strlen (tmp) < channel_b_length)
		{
			group_b = atoi (tmp);
			channel_b = strtok (NULL, "\0");
		}
	}
	
	dgs_helper_get_nid_tsid_sid_by_name (channel_a, nid_a, tsid_a, sid_a, &count_a, group_a);
	if (count_a == 0)
	{
		log_add ("Cannot found channel '%s'", channel_a);
		return;
	}
	dgs_helper_get_nid_tsid_sid_by_name (channel_b, nid_b, tsid_b, sid_b, &count_b, group_b);
	if (count_b == 0)
	{
		log_add ("Cannot found channel '%s'", channel_b);
		return;
	}
	
	alias_t alias;
	alias.count = 0;
	for (i=0; i<count_a; i++)
	{
		if (alias.count > 31) break;
		alias.nid[alias.count] = nid_a[i];
		alias.tsid[alias.count] = tsid_a[i];
		alias.sid[alias.count] = sid_a[i];
		alias.count++;
	}
	for (i=0; i<count_b; i++)
	{
		if (alias.count > 31) break;
		alias.nid[alias.count] = nid_b[i];
		alias.tsid[alias.count] = tsid_b[i];
		alias.sid[alias.count] = sid_b[i];
		alias.count++;
	}
	
	if (_aliases_get_id (aliases, &alias) == -2)
	{
		log_add ("Conflict with existing aliases");
		return;
	}
	
	fd = fopen (filename, "a");
	if (fd == NULL) fd = fopen (filename, "w");
	if (fd == NULL)
	{
		log_add ("Cannot open file '%s' for append data", filename);
		return;
	}
	
	if (alias.count > 1)
	{
		char tmp[256];
		sprintf (tmp, "# %s => %s\n", channel_a, channel_b);
		fwrite (tmp, strlen (tmp), 1, fd);
		for (i=0; i<alias.count; i++)
		{
			if (i != 0) fwrite (",", 1, 1, fd);
			sprintf (tmp, "%d|%d|%d", alias.nid[i], alias.tsid[i], alias.sid[i]);
			fwrite (tmp, strlen (tmp), 1, fd);
		}
		fwrite ("\n", 1, 1, fd);
		fflush (fd);
	}
	
	fclose (fd);
}
