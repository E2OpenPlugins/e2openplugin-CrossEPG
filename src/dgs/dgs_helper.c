#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "db/db.h"
#include "ch/ch.h"

#include "../common.h"
#include "../common/core/log.h"

#include "dgs.h"
#include "dgs_helper.h"

int _dgs_helper_get_channel_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	int *ch_id = (int*)p_data;
	*ch_id = atoi (p_fields[0]);
	return 0;
}

int dgs_helper_get_channel (int nid, int tsid, int sid)
{
	int ret = 0;
	int res;
    char *errmsg = 0;
	char sql[1024];

	if (dgs_getdb () == NULL) return 0;

	sprintf (sql,
			"SELECT	id \
			 FROM	channelinfo \
			 WHERE	org_network_id == %d AND \
					service_id == %d AND \
					ts_id == %d", nid, sid, tsid);
				
	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_channel_callback, &ret, &errmsg);
	
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
	
	return ret;
}

int _dgs_helper_get_channel_name_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	char *name = (char*)p_data;
	if ((strlen (p_fields[0]) > 0) && (strlen (p_fields[0]) < 256))
			strcpy (name, p_fields[0]);
	return 0;
}

char *dgs_helper_get_channel_name (int nid, int tsid, int sid)
{
	int res;
	char *errmsg = 0;
	char sql[1024];
	static char name[256];
	name[0] = '\0';
	
	if (dgs_getdb () == NULL) return name;

	sprintf (sql,
			"SELECT	fullname \
			 FROM	channelinfo \
			 WHERE	org_network_id == %d AND \
					service_id == %d AND \
					ts_id == %d", nid, sid, tsid);

	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_channel_name_callback, &name, &errmsg);
	
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
	
	return name;
}

char *dgs_helper_get_channel_name_by_id (int id)
{
	int res;
	char *errmsg = 0;
	char sql[1024];
	static char name[256];
	name[0] = '\0';
	
	if (dgs_getdb () == NULL) return name;

	sprintf (sql,
			"SELECT	fullname \
			 FROM	channelinfo \
			 WHERE	id == %d", id);

	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_channel_name_callback, &name, &errmsg);
	
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
	
	return name;
}

typedef struct _dgs_helper_get_channel_s
{
	int		*nid;
	int 	*tsid;
	int 	*sid;
} _dgs_helper_get_channel_t;

int _dgs_helper_get_nid_tsid_sid_by_id_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	_dgs_helper_get_channel_t *data = (_dgs_helper_get_channel_t*)p_data;
	
	*(data->nid) = atoi (p_fields[0]);
	*(data->tsid) = atoi (p_fields[1]);
	*(data->sid) = atoi (p_fields[2]);

	return 0;
}

void dgs_helper_get_nid_tsid_sid_by_id (int id, int *nid, int *tsid, int *sid)
{
	int res;
	char *errmsg = 0;
	char sql[1024];
	_dgs_helper_get_channel_t ret;
	
	ret.nid = nid;
	ret.tsid = tsid;
	ret.sid = sid;
	
	if (dgs_getdb () == NULL) return;

	sprintf (sql, "SELECT org_network_id, ts_id, service_id FROM channelinfo WHERE id = %d LIMIT 1", id);

	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_nid_tsid_sid_by_id_callback, &ret, &errmsg);
	
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
}

typedef struct _dgs_helper_get_channels_s
{
	int		*count;
	int 	*nid;		// array
	int 	*tsid;		// array
	int 	*sid;		// array
	int		*ids;		// array
} _dgs_helper_get_channels_t;

int _dgs_helper_get_nid_tsid_sid_by_name_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	_dgs_helper_get_channels_t *data = (_dgs_helper_get_channels_t*)p_data;
	if (*data->count < 32)
	{
		data->nid[*data->count] = atoi (p_fields[0]);
		data->tsid[*data->count] = atoi (p_fields[1]);
		data->sid[*data->count] = atoi (p_fields[2]);
		*data->count = *data->count + 1;
	}

	return 0;
}

void dgs_helper_get_nid_tsid_sid_by_name (char *name, int *nid, int *tsid, int *sid, int *count, int group)
{
	int res;
	char *errmsg = 0;
	char sql[1024];
	_dgs_helper_get_channels_t ret;
	
	*count = 0;
	
	ret.nid = nid;
	ret.tsid = tsid;
	ret.sid = sid;
	ret.count = count;
	
	if (dgs_getdb () == NULL) return;

	if (group == -1)
		sprintf (sql, "SELECT org_network_id, ts_id, service_id FROM channelinfo WHERE fullname LIKE '%s'", name);
	else
		sprintf (sql, "SELECT channelinfo.org_network_id, channelinfo.service_id, channelinfo.ts_id FROM channelinfo INNER JOIN FAVListInfo ON channelinfo.id = FAVListInfo.ch_id WHERE channelinfo.fullname LIKE '%s' AND FAVListInfo.grp_id = '%d'", name, group);

	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_nid_tsid_sid_by_name_callback, &ret, &errmsg);
	
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
}

int _dgs_helper_get_channels_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	int i, nid, tsid, sid;
	_dgs_helper_get_channels_t *args = (_dgs_helper_get_channels_t*)p_data;

	nid = atoi (p_fields[1]);
	tsid = atoi (p_fields[3]);
	sid = atoi (p_fields[2]);
	
	for (i=0; i<*args->count; i++)
		if ((args->nid[i] == nid) && (args->tsid[i] == tsid) && (args->sid[i] == sid))
		{
			args->ids[i] = atoi (p_fields[0]);
			break;
		}
	
	return 0;
}

void dgs_helper_get_channels (int *nid, int *tsid, int *sid, int *ids, int count, int *groups, int groups_count)
{
	int res;
    char *errmsg = 0;
	char sql[16*1024];
	int i;

	_dgs_helper_get_channels_t args;
	args.nid = nid;
	args.tsid = tsid;
	args.sid = sid;
	args.ids = ids;
	*args.count = count;
	
	if (dgs_getdb () == NULL) return;
 	
	if (groups_count > 0)
	{
		char filter[16*1024];
		char *tmp = filter;
		for (i=0; i<groups_count; i++)
		{
			if (i == (groups_count-1))
				sprintf (tmp, "FAVListInfo.grp_id = '%d'", groups[i]);
			else
				sprintf (tmp, "FAVListInfo.grp_id = '%d' OR ", groups[i]);
				
			tmp += strlen (tmp);
		}
		sprintf (sql,
				"SELECT	channelinfo.id, channelinfo.org_network_id, channelinfo.service_id, channelinfo.ts_id FROM channelinfo INNER JOIN FAVListInfo ON channelinfo.id = FAVListInfo.ch_id WHERE %s", filter);
	}
	else
	{
		sprintf (sql,
				"SELECT	    channelinfo.id, channelinfo.org_network_id, channelinfo.service_id, channelinfo.ts_id \
				 FROM	    channelinfo");
	}

	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_channels_callback, &args, &errmsg);
	
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
	
	return;
}

int _dgs_helper_get_current_group_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	int *grp_id = (int*)p_data;
	*grp_id = atoi (p_fields[0]);
	return 0;
}

int dgs_helper_get_current_group (int ch_id)
{
	int ret = -1;
	int res;
    char *errmsg = 0;
	char sql[1024];

	if (dgs_getdb () == NULL) return -1;

	sprintf (sql,
			"SELECT	grp_id \
			FROM	FAVListInfo \
			WHERE	ch_id == %d LIMIT 1", ch_id);

	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_current_group_callback, &ret, &errmsg);
		
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
	
	return ret;
}

int _dgs_helper_get_option_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{	
	char **value = (char**)p_data;
	strcpy (*value, p_fields[0]);
	return 0;
}

char *dgs_helper_get_menu_language ()
{
	char *ret = _malloc (64);
	int res;
    char *errmsg = 0;
	char sql[1024];

	ret[0] = '\0';
	if (dgs_getdb () == NULL) return ret;

	sprintf (sql,
			"SELECT	value \
			FROM	options \
			WHERE	name == 'menu_language' LIMIT 1");

	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_option_callback, &ret, &errmsg);
		
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
	
	return ret;
}

char *dgs_helper_get_aspect_ratio ()
{
	char *ret = _malloc (64);
	int res;
    char *errmsg = 0;
	char sql[1024];

	ret[0] = '\0';
	if (dgs_getdb () == NULL) return ret;

	sprintf (sql, "SELECT value FROM options WHERE name == 'aspect_ratio' LIMIT 1");

	res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_option_callback, &ret, &errmsg);
		
	if (res != SQLITE_OK)
		log_add ("SQL error: %s", errmsg);
	
	return ret;
}

bool dgs_helper_get_daylight_saving ()
{
	static bool is_cached = false;
	static bool cache = false;
	int res;
	char *errmsg = 0;
	char sql[1024];

	if (!is_cached)
	{
		is_cached = true;
		if (dgs_getdb () == NULL) return false;
		char *ret = _malloc (16);
		ret[0] = '\0';
		
		sprintf (sql,
				"SELECT	value \
				FROM	options \
				WHERE	name == 'daylight_saving' LIMIT 1");
			
		res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_get_option_callback, &ret, &errmsg);
		
		if (res != SQLITE_OK)
			log_add ("SQL error: %s", errmsg);
		
		if (strlen (ret) == 2)
			if (memcmp (ret, "on", 2) == 0)
				cache = true;
		
		_free (ret);
	}
	
	return cache;
}

time_t dgs_helper_adjust_daylight (time_t value)
{
	return value + (dgs_helper_get_daylight_saving () * (60 * 60));
}

void dgs_helper_commander (char *command)
{
	FILE *fd;
	fd = fopen ("/dev/commander", "w");
	
	if (fd != NULL)
	{
		fwrite (command, strlen(command), 1, fd);
		fclose (fd);
	}
	else
	{
		log_add ("Cannot write in /dev/commander");
		log_add ("Error number %d", errno);
	}
}

int _dgs_helper_add_scheduler_callback (void *p_data, int num_fields, char **p_fields, char **p_col_names)
{
	int *count = (int*)p_data;
	*count = *count + 1;
	return 0;
}

bool dgs_helper_add_scheduler (int channel_id, time_t start_time, int length, int type, char *name)
{
	FILE *fd;
	char cmd[256];
	char data[4096];
	bool ret = false;
	pid_t child_pid;

	sprintf (cmd, "web_add_schedule %d %d %d %d 0 1 0 \\\"%s\\\"\n", channel_id, (int)start_time, length, type, name);
	memset (data, 0, 4096);
	child_pid = fork();
	if (child_pid == 0)
	{
		usleep (100000);
		FILE *fd;
		fd = fopen ("/dev/commander", "w");
		if (fd != NULL)
		{
			fwrite (cmd, strlen (cmd), 1, fd);
			fclose (fd);
		}
		sleep (3);
		exit (0);
	}
	else if (child_pid > 0)
	{
		fd = fopen ("/dev/weboutput", "r");
		if (fd != NULL)
		{
			int status;
			waitpid (child_pid, &status, 0);
			
			fread (data, 4096, 1, fd);
			fclose (fd);
			if (memcmp (data, "true", 4) == 0) ret = true;
		}
		else log_add ("Cannot read from /dev/weboutput");
	}
	else
		log_add ("Cannot fork process!");
	
	return ret;
}

void dgs_helper_del_scheduler (int scheduler_id)
{
	char sql[1024];
	char *errmsg = 0;
	int res, i;
	char cmd[256];
	sprintf (cmd, "web_delete_schedule %d\n", scheduler_id);
	dgs_helper_commander (cmd);
	
	for (i=0; i<10; i++)
	{
		int count=0;
		sprintf (sql, "SELECT * FROM scheduledinfo WHERE sch_id = %d", scheduler_id);
		
		res = sqlite3_exec (dgs_getdb (), sql, _dgs_helper_add_scheduler_callback, &count, &errmsg);
		if (res != SQLITE_OK)
			log_add ("SQL error: %s", errmsg);
			
		if (count == 0) break;
		sleep (1);
	}
}

void dgs_helper_live_restore ()
{
#ifndef STANDALONE
	bool setted = false;
	bool pillar_box = false;
	bool zoom_box = false;
	char *ratio = dgs_helper_get_aspect_ratio ();
	if (strlen (ratio) == strlen ("pillar_box"))
		if (memcmp (ratio, "pillar_box", strlen ("pillar_box")) == 0)
			pillar_box = true;
			
	if (strlen (ratio) == strlen ("4:3_zoom"))
		if (memcmp (ratio, "4:3_zoom", strlen ("4:3_zoom")) == 0)
			zoom_box = true;
	
	if (pillar_box)
	{
		vid_stream_info_t stream_info;
		if (ch_watching_video_stream_info (ch_mode_live, &stream_info) == 0)
			if (stream_info.aspect_ratio == vid_aspect_ratio_4to3)
			{
				dgs_helper_commander ("vid_pig 104 0 512 576\n");
				setted = true;
			}
	}
	
	if (zoom_box)
	{
		vid_stream_info_t stream_info;
		if (ch_watching_video_stream_info (ch_mode_live, &stream_info) == 0)
			if (stream_info.aspect_ratio == vid_aspect_ratio_4to3)
			{
				dgs_helper_commander ("vid_pig 0 -80 720 736\n");
				setted = true;
			}
	}
	
	if (!setted) dgs_helper_commander ("vid_pig 0 0 720 576\n");
		
	_free (ratio);
#endif
}

void dgs_helper_live_boxed ()
{
	dgs_helper_commander ("vid_pig 460 50 236 210\n");
}

void dgs_helper_power_on ()
{
	dgs_helper_commander ("web_warm_on\n");
	sleep (2);
}

void dgs_helper_power_off ()
{
	dgs_helper_commander ("web_warm_off\n");
	sleep (2);
}

// return 0 = power on
// return -1 = power off
int dgs_helper_power_status ()
{
	char tmp[256];
	FILE *fd;
	
	dgs_helper_commander ("web_status\n");
	sleep (1);
	fd = fopen ("/tmp/status", "r");
	if (fd != NULL)
	{
		fseek (fd, 0, SEEK_END);
		int len = ftell (fd);
		fseek (fd, 0, SEEK_SET);
		fread (tmp, len, 1, fd);
		fclose (fd);
		
		if (len == 8)
			if (memcmp (tmp, "warm_off", 8) == 0)
				return -1;
	}
	
	return 0;
}
