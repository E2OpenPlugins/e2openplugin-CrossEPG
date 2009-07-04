#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#ifdef STANDALONE
#include <sqlite3.h>
#else
#include "db/db.h"
#include "directory.h"
#endif

#include "../../common.h"
#include "log.h"

#include "../../dgs/dgs.h"

#include "config.h"

#ifdef STANDALONE
char dir_plugin[256];
void config_set_dir_plugin (char *value) { strcpy (dir_plugin, value); }
#endif

static int sync_hours;
static char log_file[256];
static char dwnl_log_file[256];
static char sync_log_file[256];
static char launcher_log_file[256];
//static char db_headers_file[256];
//static char db_descriptors_file[256];
static char db_root[256];
static char dwnl_file[256];
static char sync_file[256];
static char home_directory[256];
static int sync_groups[64];
static int visible_groups[64];
static char otv_provider[64][256];
static char skin[256];
static char launcher_red[256];
static char launcher_green[256];
static char launcher_yellow[256];
static char launcher_blue[256];
static char launcher_long_red[256];
static char launcher_long_green[256];
static char launcher_long_yellow[256];
static char launcher_long_blue[256];
static int start_screen;
static int boot_action;
static int cron_action;
static int cron_hour;
static int channel_sleep;
static int show_title;

int   config_get_sync_hours				() { return sync_hours;				}
char *config_get_log_file				() { return log_file;				}
char *config_get_dwnl_log_file			() { return dwnl_log_file;			}
char *config_get_sync_log_file			() { return sync_log_file;			}
char *config_get_launcher_log_file		() { return launcher_log_file;		}
char *config_get_db_root				() { return db_root;				}
char *config_get_dwnl_file				() { return dwnl_file;				}
char *config_get_sync_file				() { return sync_file;				}
char *config_get_home_directory			() { return home_directory;			}
int  *config_get_sync_groups			() { return sync_groups;			}
int  *config_get_visible_groups			() { return visible_groups;			}
char *config_get_skin					() { return skin;					}
char *config_get_otv_provider			(int id) { return otv_provider[id]; }
char *config_get_launcher_red			() { return launcher_red;			}
char *config_get_launcher_green			() { return launcher_green;			}
char *config_get_launcher_yellow		() { return launcher_yellow;		}
char *config_get_launcher_blue			() { return launcher_blue;			}
char *config_get_launcher_long_red		() { return launcher_long_red;		}
char *config_get_launcher_long_green	() { return launcher_long_green;	}
char *config_get_launcher_long_yellow	() { return launcher_long_yellow;	}
char *config_get_launcher_long_blue		() { return launcher_long_blue;		}
int   config_get_start_screen			() { return start_screen;			}
int   config_get_boot_action			() { return boot_action;			}
int   config_get_cron_action			() { return cron_action;			}
int   config_get_cron_hour				() { return cron_hour;				}
int   config_get_channel_sleep			() { return channel_sleep;			}
int   config_get_show_title				() { return show_title;				}

void config_set_db_root					(char *value) { strcpy (db_root					, value); }
void config_set_log_file				(char *value) { strcpy (log_file				, value); }
void config_set_dwnl_log_file			(char *value) { strcpy (dwnl_log_file			, value); }
void config_set_sync_log_file			(char *value) { strcpy (sync_log_file			, value); }
void config_set_launcher_log_file		(char *value) { strcpy (launcher_log_file		, value); }
void config_set_skin					(char *value) { strcpy (skin					, value); }
void config_set_launcher_red			(char *value) { strcpy (launcher_red			, value); }
void config_set_launcher_green			(char *value) { strcpy (launcher_green			, value); }
void config_set_launcher_yellow			(char *value) { strcpy (launcher_yellow			, value); }
void config_set_launcher_blue			(char *value) { strcpy (launcher_blue			, value); }
void config_set_launcher_long_red		(char *value) { strcpy (launcher_long_red		, value); }
void config_set_launcher_long_green		(char *value) { strcpy (launcher_long_green		, value); }
void config_set_launcher_long_yellow	(char *value) { strcpy (launcher_long_yellow	, value); }
void config_set_launcher_long_blue		(char *value) { strcpy (launcher_long_blue		, value); }
void config_set_start_screen			(int value) { start_screen = value; }
void config_set_boot_action				(int value) { boot_action = value; }
void config_set_cron_action				(int value) { cron_action = value; }
void config_set_cron_hour				(int value) { cron_hour = value; }
void config_set_channel_sleep			(int value) { channel_sleep = value; }
void config_set_show_title				(int value) { show_title = value; }
void config_set_otv_provider			(int id, char *value) { strcpy (otv_provider[id] , value); }

void config_set_sync_hours				(int value) { sync_hours = value; }
void config_set_sync_groups				(int id, int value) { sync_groups[id] = value; }
void config_set_visible_groups			(int id, int value) { visible_groups[id] = value; }

static char *config_trim_spaces (char *text)
{
	char *tmp = text;
	while (tmp[0] == ' ') tmp++;
	while (strlen (tmp) > 1)
		if (tmp[strlen (tmp) - 1] == ' ') tmp[strlen (tmp) - 1] = '\0';
		else break;
	
	if (tmp[0] == ' ') tmp[0] = '\0';
	return tmp;
}

bool config_read ()
{
	FILE *fd = NULL;
	char line[512];
	char key[256];
	char value[256];
	char config_file[512];
	int i;

	sync_hours = DEFAULT_SYNC_HOURS;
	strcpy (log_file, DEFAULT_LOG_FILE);
	strcpy (dwnl_log_file, DEFAULT_DWNL_LOG_FILE);
	strcpy (sync_log_file, DEFAULT_SYNC_LOG_FILE);
	strcpy (launcher_log_file, DEFAULT_LAUNCHER_LOG_FILE);
	strcpy (db_root, DEFAULT_DB_ROOT);
	strcpy (home_directory, DEFAULT_HOME_DIRECTORY);
	//strcpy (otv_provider[0], DEFAULT_OTV_PROVIDER);
	
	memset (launcher_red, 0, 256);
	memset (launcher_green, 0, 256);
	memset (launcher_yellow, 0, 256);
	memset (launcher_blue, 0, 256);
	memset (launcher_long_red, 0, 256);
	memset (launcher_long_green, 0, 256);
	memset (launcher_long_yellow, 0, 256);
	memset (launcher_long_blue, 0, 256);
	
	strcpy (skin, "default");
	
	for (i=0; i<64; i++)
	{
		sync_groups[i] = 0;
		visible_groups[i] = 0;
		otv_provider[i][0] = '\0';
	}
	
	channel_sleep = 1;
	start_screen = 1;
	boot_action = 2;
	cron_action = 3;
	cron_hour = 3;
	show_title = 1;
	
	sprintf (dwnl_file, "%s/%s", dir_plugin, DEFAULT_DWNL_FILE);
	sprintf (sync_file, "%s/%s", dir_plugin, DEFAULT_SYNC_FILE);
	
	sprintf (config_file, "%s/%s", dir_plugin, DEFAULT_CONFIG_FILE);

	fd = fopen (config_file, "r");
	if (!fd) 
		return false;

	while (fgets (line, sizeof(line), fd)) 
	{
		char *tmp_key, *tmp_value;
		
		memset (key, 0, sizeof (key));
		memset (value, 0, sizeof (value));
		
		if (sscanf (line, "%[^#=]=%s\n", key, value) != 2)
			continue;
		
		tmp_key = config_trim_spaces (key);
		tmp_value = config_trim_spaces (value);
		
		if (strcmp ("sync_hours", tmp_key) == 0)
			sync_hours = atoi (tmp_value);
		else if (strcmp ("db_root", tmp_key) == 0)
			strcpy (db_root, tmp_value);
		else if (strcmp ("dwnl_file", tmp_key) == 0)
			strcpy (dwnl_file, tmp_value);
		else if (strcmp ("sync_file", tmp_key) == 0)
			strcpy (sync_file, tmp_value);
		else if (strcmp ("log_file", tmp_key) == 0)
			strcpy (log_file, tmp_value);
		else if (strcmp ("dwnl_log_file", tmp_key) == 0)
			strcpy (dwnl_log_file, tmp_value);
		else if (strcmp ("sync_log_file", tmp_key) == 0)
			strcpy (sync_log_file, tmp_value);
		else if (strcmp ("launcher_log_file", tmp_key) == 0)
			strcpy (launcher_log_file, tmp_value);
		else if (strcmp ("home_directory", tmp_key) == 0)
			strcpy (home_directory, tmp_value);
		else if (strcmp ("sync_groups", tmp_key) == 0)
		{
			int count = 0;
			char* tmp = strtok (tmp_value, "|");
			while ((tmp != NULL) && (count < 64))
			{
				sync_groups[count] = atoi (tmp);
				tmp = strtok (NULL, "|");
				count++;
			}
		}		
		else if (strcmp ("visible_groups", tmp_key) == 0)
		{
			int count = 0;
			char* tmp = strtok (tmp_value, "|");
			while ((tmp != NULL) && (count < 64))
			{
				visible_groups[count] = atoi (tmp);
				tmp = strtok (NULL, "|");
				count++;
			}
		}		
		else if (strcmp ("otv_provider", tmp_key) == 0)
		{
			int count = 0;
			char* tmp = strtok (tmp_value, "|");
			while ((tmp != NULL) && (count < 64))
			{
				strcpy (otv_provider[count], tmp);
				tmp = strtok (NULL, "|");
				count++;
			}
		}
		else if (strcmp ("skin", tmp_key) == 0)
			strcpy (skin, tmp_value);
		else if (strcmp ("launcher_red", tmp_key) == 0)
			strcpy (launcher_red, tmp_value);
		else if (strcmp ("launcher_green", tmp_key) == 0)
			strcpy (launcher_green, tmp_value);
		else if (strcmp ("launcher_yellow", tmp_key) == 0)
			strcpy (launcher_yellow, tmp_value);
		else if (strcmp ("launcher_blue", tmp_key) == 0)
			strcpy (launcher_blue, tmp_value);
		else if (strcmp ("launcher_long_red", tmp_key) == 0)
			strcpy (launcher_long_red, tmp_value);
		else if (strcmp ("launcher_long_green", tmp_key) == 0)
			strcpy (launcher_long_green, tmp_value);
		else if (strcmp ("launcher_long_yellow", tmp_key) == 0)
			strcpy (launcher_long_yellow, tmp_value);
		else if (strcmp ("launcher_long_blue", tmp_key) == 0)
			strcpy (launcher_long_blue, tmp_value);
		else if (strcmp ("start_screen", tmp_key) == 0)
			start_screen = atoi (tmp_value);
		else if (strcmp ("boot_action", tmp_key) == 0)
			boot_action = atoi (tmp_value);
		else if (strcmp ("cron_action", tmp_key) == 0)
			cron_action = atoi (tmp_value);
		else if (strcmp ("cron_hour", tmp_key) == 0)
			cron_hour = atoi (tmp_value);
		else if (strcmp ("channel_sleep", tmp_key) == 0)
			channel_sleep = atoi (tmp_value);
		else if (strcmp ("show_title", tmp_key) == 0)
			show_title = atoi (tmp_value);
	}
	
	/* strip extra slashes from home and db path */
	while (home_directory[strlen (home_directory) - 1] == '/') home_directory[strlen (home_directory) - 1] = '\0';
	while (db_root[strlen (db_root) - 1] == '/') db_root[strlen (db_root) - 1] = '\0';
	
	fclose (fd);
	
	return true;
}

bool config_save ()
{
	FILE *fd;
	int i;
	char config_file[512];
	char config_tmp[32*1024];
	char *tmp = config_tmp;
	char groups[512];
	char providers[1024*32];
	char *tmp_groups = groups;
	char *tmp_providers = providers;
	sprintf (tmp, "db_root=%s\n", db_root);
	tmp += strlen (tmp);
	sprintf (tmp, "log_file=%s\n", log_file);
	tmp += strlen (tmp);
	sprintf (tmp, "dwnl_log_file=%s\n", dwnl_log_file);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_log_file=%s\n", launcher_log_file);
	tmp += strlen (tmp);
	sprintf (tmp, "sync_log_file=%s\n", sync_log_file);
	tmp += strlen (tmp);
	sprintf (tmp, "home_directory=%s\n", home_directory);
	tmp += strlen (tmp);
	sprintf (tmp, "sync_hours=%d\n", sync_hours);
	tmp += strlen (tmp);
	sprintf (tmp, "skin=%s\n", skin);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_red=%s\n", launcher_red);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_green=%s\n", launcher_green);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_yellow=%s\n", launcher_yellow);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_blue=%s\n", launcher_blue);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_long_red=%s\n", launcher_long_red);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_long_green=%s\n", launcher_long_green);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_long_yellow=%s\n", launcher_long_yellow);
	tmp += strlen (tmp);
	sprintf (tmp, "launcher_long_blue=%s\n", launcher_long_blue);
	tmp += strlen (tmp);
	sprintf (tmp, "start_screen=%d\n", start_screen);
	tmp += strlen (tmp);
	sprintf (tmp, "boot_action=%d\n", boot_action);
	tmp += strlen (tmp);
	sprintf (tmp, "cron_action=%d\n", cron_action);
	tmp += strlen (tmp);
	sprintf (tmp, "cron_hour=%d\n", cron_hour);
	tmp += strlen (tmp);
	sprintf (tmp, "show_title=%d\n", show_title);
	tmp += strlen (tmp);
	
	groups[0] = '\0';
	for (i=0; i<64; i++)
	{
		if (sync_groups[i] != 0)
		{
			sprintf (tmp_groups, "%d|", sync_groups[i]);
			tmp_groups += strlen (tmp_groups);
		}
	}
	if (strlen (groups) == 0) sprintf (tmp, "sync_groups=\n");
	else
	{
		groups[strlen (groups) - 1] = '\0';
		sprintf (tmp, "sync_groups=%s\n", groups);
	}
	tmp += strlen (tmp);

	groups[0] = '\0';
	tmp_groups = groups;
	for (i=0; i<64; i++)
	{
		if (visible_groups[i] != 0)
		{
			sprintf (tmp_groups, "%d|", visible_groups[i]);
			tmp_groups += strlen (tmp_groups);
		}
	}
	if (strlen (groups) == 0) sprintf (tmp, "visible_groups=\n");
	else
	{
		groups[strlen (groups) - 1] = '\0';
		sprintf (tmp, "visible_groups=%s\n", groups);
	}
	tmp += strlen (tmp);
	
	providers[0] = '\0';
	for (i=0; i<64; i++)
	{
		if (strlen (otv_provider[i]) > 0)
		{
			sprintf (tmp_providers, "%s|", otv_provider[i]);
			tmp_providers += strlen (tmp_providers);
		}
	}
	if (strlen (providers) > 0)
	{
		providers[strlen (providers) - 1] = '\0';
		sprintf (tmp, "otv_provider=%s\n", providers);
	}

	tmp += strlen (tmp);
	sprintf (tmp, "channel_sleep=%d\n", channel_sleep);
	
	sprintf (config_file, "%s/%s", dir_plugin, DEFAULT_CONFIG_FILE);
	
	fd = fopen (config_file, "w");
	if (!fd) 
		return false;
	
	fwrite (config_tmp, strlen (config_tmp), 1, fd);
	fflush (fd);
	fclose (fd);
	return true;
}
