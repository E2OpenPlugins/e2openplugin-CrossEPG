#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef STANDALONE
void config_set_dir_plugin (char *value);
#endif

bool config_read ();
bool config_save ();
int config_get_sync_hours ();
char *config_get_log_file ();
char *config_get_dwnl_log_file ();
char *config_get_sync_log_file ();
char *config_get_launcher_log_file ();
char *config_get_db_root ();
char *config_get_dwnl_file ();
char *config_get_sync_file ();
char *config_get_home_directory ();
int *config_get_sync_groups ();
int *config_get_visible_groups ();
char *config_get_otv_provider (int id);
char *config_get_skin ();
char *config_get_launcher_red ();
char *config_get_launcher_green ();
char *config_get_launcher_yellow ();
char *config_get_launcher_blue ();
char *config_get_launcher_long_red ();
char *config_get_launcher_long_green ();
char *config_get_launcher_long_yellow ();
char *config_get_launcher_long_blue ();
int config_get_start_screen ();
int config_get_boot_action ();
int config_get_cron_action ();
int config_get_cron_hour ();
int config_get_channel_sleep ();
int config_get_show_title ();

void config_set_db_root (char *value);
void config_set_log_file (char *value);
void config_set_dwnl_log_file (char *value);
void config_set_sync_log_file (char *value);
void config_set_launcher_log_file (char *value);
void config_set_otv_provider (int id, char *value);
void config_set_skin (char *value);
void config_set_launcher_red (char *value);
void config_set_launcher_green (char *value);
void config_set_launcher_yellow (char *value);
void config_set_launcher_blue (char *value);
void config_set_launcher_long_red (char *value);
void config_set_launcher_long_green (char *value);
void config_set_launcher_long_yellow (char *value);
void config_set_launcher_long_blue (char *value);
void config_set_start_screen (int value);
void config_set_boot_action (int value);
void config_set_cron_action (int value);
void config_set_cron_hour (int value);
void config_set_channel_sleep (int value);
void config_set_show_title (int value);

void config_set_sync_hours (int value);
void config_set_sync_groups (int id, int value);
void config_set_visible_groups (int id, int value);

#endif // _CONFIG_H_
