#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sqlite3.h>

#include "../common.h"

#include "../common/core/log.h"
#include "../common/core/config.h"
#include "../common/epgdb/epgdb.h"
#include "dgs.h"
#include "dgs_channels.h"
#include "dgs_helper.h"
#include "dgs_scheduler.h"
#include "scheduler/scheduler.h"

static char homedir[256]; // = DEFAULT_HOME_DIRECTORY;
static char plugindir[256];

static volatile bool kill_request;

static bool crossepg_state;
static bool sync_state;
static bool download_state;

void sigint_handler (int sig) { kill_request = true; }

bool savepid ()
{
	char pid_txt[256];
	pid_t pid = getpid ();
	FILE *fd = fopen ("/tmp/crossepgd.pid", "w");
	if (fd == NULL) return false;
	sprintf (pid_txt, "%d", (int)pid);
	fwrite (pid_txt, strlen (pid_txt), 1, fd);
	fclose (fd);
	return true;
}

void delpid ()
{
	unlink ("/tmp/crossepgd.pid");
}

bool check_crossepg ()
{
	FILE *fd = fopen ("/tmp/crossepg.pid", "r");
	if (fd == NULL) return false;
	fclose (fd);
	return true;
}

bool check_sync ()
{
	FILE *fd = fopen ("/tmp/crossepg.sync.pid", "r");
	if (fd == NULL) return false;
	fclose (fd);
	return true;
}

bool check_download ()
{
	FILE *fd = fopen ("/tmp/crossepg.downloader.pid", "r");
	if (fd == NULL) return false;
	fclose (fd);
	return true;
}

void scheduler_sync ()
{
	bool changed = false;
	dgs_scheduler_init ();
	scheduler_init ();
	
	if (!dgs_scheduler_read ())
	{
		dgs_scheduler_clean ();
		return;
	}
	if (!scheduler_load (config_get_db_root ()))
	{
		scheduler_clean ();
		dgs_scheduler_clean ();
		return;
	}
	
	scheduler_t *tmp2;
	dgs_scheduler_t *tmp = dgs_scheduler_get_first ();
	
	while (tmp != NULL)
	{
		bool exist = false;
		tmp2 = scheduler_get_first ();
		while (tmp2 != NULL)
		{
			if ((tmp2->start_time == tmp->start_time) && (tmp2->length == tmp->length) && ((tmp2->type == 0) || (tmp2->type == 1)))
			{
				exist = true;
				break;
			}
			tmp2 = tmp2->next;
		}
		if (!exist)
		{
			scheduler_add (tmp->ch_id, tmp->start_time, tmp->length, tmp->mode, tmp->name, false);
			changed = true;
		}
		tmp = tmp->next;
	}
	
	tmp2 = scheduler_get_first ();
	while (tmp2 != NULL)
	{
		if ((tmp2->type == 0) || (tmp2->type == 1))
		{
			bool exist = false;
			tmp = dgs_scheduler_get_first ();
			while (tmp != NULL)
			{
				if ((tmp2->start_time == tmp->start_time) && (tmp2->length == tmp->length))
				{
					exist = true;
					break;
				}
				tmp = tmp->next;
			}
			if (!exist)
			{
				scheduler_del (tmp2->channel_id, tmp2->start_time, tmp2->length, tmp2->type, false);
				changed = true;
			}
		}
		tmp2 = tmp2->next;
	}
	
	if (changed)
	{
		if (scheduler_save (config_get_db_root ())) log_add ("Saved scheduler list");
		else log_add ("Error saving scheduler list");
	}
	scheduler_clean ();
	dgs_scheduler_clean ();
}

int main (int argc, char **argv)
{
	static struct sigaction act;
	struct stat attrib;
	int c;
	time_t now;
	time_t config_time;
	int exec_request = 0; // 1 download - 2 sync - 3 download and sync
	int cron_hour = 0;
	int cron_min = 0;
	int last_day = -1;
	char configfile[256];
	int scheduler_check_count = 0;
	
	opterr = 0;
	
	sprintf (homedir, DEFAULT_HOME_DIRECTORY);
	sprintf (plugindir, "/var/plug-in");
	
	log_open (NULL, "CrossEPG Daemon");
	
	while ((c = getopt (argc, argv, "l:p:")) != -1)
	{
		switch (c)
		{
			case 'l':
				strcpy (homedir, optarg);
				break;
			case 'p':
				strcpy (plugindir, optarg);
				break;
			case '?':
				printf ("Usage:\n");
				printf ("  ./crossepgd [options]\n");
				printf ("Options:\n");
				printf ("  -l homedir       home directory\n");
				printf ("                   default: %s\n", homedir);
				printf ("  -p plugindir     plugin directory\n");
				printf ("                   default: %s\n", plugindir);
				printf ("  -h               show this help\n");
				return 0;
		}
	}
	
	while (homedir[strlen (homedir) - 1] == '/') homedir[strlen (homedir) - 1] = '\0';
	while (plugindir[strlen (plugindir) - 1] == '/') plugindir[strlen (plugindir) - 1] = '\0';
	
	config_set_dir_plugin (plugindir);
	
	if (!config_read ())
	{
		log_add ("Error loading crossepg configuration");
		return 0;
	}
	
	sprintf (configfile, "%s/crossepg.config", plugindir);
	stat (configfile, &attrib);
	config_time = attrib.st_mtime;
	
	kill_request = false;
	
	act.sa_handler = sigint_handler;
	act.sa_flags = 0;
	sigfillset (&(act.sa_mask));      /* create full set of signals */
	
	sigaction (SIGINT, &act, NULL );
	sigaction (SIGHUP, &act, NULL );
	sigaction (SIGTERM, &act, NULL );
	sigaction (SIGKILL, &act, NULL );
	
	if (!savepid ())
	{
		log_add ("Error creating /tmp/crossepgd.pid");
		return 0;
	}
	
	if (!dgs_opendb ())
	{
		log_add ("Error opening dgs db");
		delpid ();
		return 0;
	}
	
	crossepg_state = false;
	sync_state = false;
	download_state = false;
	
	cron_hour = config_get_cron_hour ();
	exec_request = config_get_boot_action ();
	if (exec_request == 0) log_add ("No boot action");
	else if (exec_request == 1) log_add ("Boot action: download");
	else if (exec_request == 2) log_add ("Boot action: sync");
	else if (exec_request == 3) log_add ("Boot action: download and sync");
	
	while (!kill_request)
	{
		stat (configfile, &attrib);
		if (config_time != attrib.st_mtime)
		{
			log_add ("CrossEPG configuration changed");
			if (config_read ()) log_add ("Configuration reloaded");
			else log_add ("Error reloading configuration");
			config_time = attrib.st_mtime;
			cron_hour = config_get_cron_hour ();
			last_day = -1;
		}
		
		if (sync_state)
		{
			if (!check_sync ())
			{
				log_add ("Sync Plugin terminated");
				sync_state = false;
			}
		}
		else
		{
			if (check_sync ())
			{
				log_add ("Sync Plugin started");
				sync_state = true;
			}
		}
		
		if (download_state)
		{
			if (!check_download ())
			{
				log_add ("Downloader Plugin terminated");
				download_state = false;
			}
		}
		else
		{
			if (check_download ())
			{
				log_add ("Downloader Plugin started");
				download_state = true;
			}
		}
		
		if (crossepg_state)
		{
			if (!check_crossepg ())
			{
				log_add ("CrossEPG Plugin terminated");
				crossepg_state = false;
			}
		}
		else
		{
			if (check_crossepg ())
			{
				log_add ("CrossEPG Plugin started");
				crossepg_state = true;
			}
		}
		
		if (!sync_state && !download_state && !crossepg_state)
		{
			if ((exec_request == 1) || (exec_request == 3))
			{
				dgs_helper_commander ("web_execute_plugin crossepg.downloader.plugin");
				if (exec_request == 3) exec_request = 2;
				else exec_request = 0;
			}
			else if (exec_request == 2)
			{
				dgs_helper_commander ("web_execute_plugin crossepg.sync.plugin");
				exec_request = 0;
			}
			else
			{
				scheduler_check_count++;
				if (scheduler_check_count > 5)
				{
					scheduler_sync ();
					scheduler_check_count = 0;
				}
			}
		}
		
		struct tm loctime;
		now = time (NULL);
		localtime_r (&now, &loctime);
		if ((loctime.tm_hour == cron_hour) && (loctime.tm_min == cron_min) && (loctime.tm_mday != last_day))
		{
			log_add ("It's time for daily action");
			last_day = loctime.tm_mday;
			exec_request = config_get_cron_action ();
			if (exec_request == 0) log_add ("No daily action");
			else if (exec_request == 1) log_add ("Daily action: download");
			else if (exec_request == 2) log_add ("Daily action: sync");
			else if (exec_request == 3) log_add ("Daily action: download and sync");
		}
		
		sleep (1);
	}
	log_add ("Exiting");
	delpid ();
	
	dgs_closedb ();
	memory_stats ();
	log_close ();
	return 0;
}
