#ifndef _DGS_HELPER_H_
#define _DGS_HELPER_H_

int dgs_helper_get_current_group (int ch_id);
int dgs_helper_get_channel (int nid, int tsid, int sid);
char *dgs_helper_get_channel_name (int nid, int tsid, int sid);
char *dgs_helper_get_channel_name_by_id (int id);
void dgs_helper_get_nid_tsid_sid_by_name (char *name, int *nid, int *tsid, int *sid, int *count, int group);
void dgs_helper_get_nid_tsid_sid_by_id (int id, int *nid, int *tsid, int *sid);
void dgs_helper_get_channels (int *nid, int *tsid, int *sid, int *ids, int count, int *groups, int groups_count);
char *dgs_helper_get_menu_language ();
char *dgs_helper_get_aspect_ratio ();
bool dgs_helper_get_daylight_saving ();
time_t dgs_helper_adjust_daylight (time_t value);
void dgs_helper_commander (char *command);
bool dgs_helper_add_scheduler (int channel_id, time_t start_time, int length, int type, char *name);
void dgs_helper_del_scheduler (int scheduler_id);
void dgs_helper_live_boxed ();
void dgs_helper_live_restore ();
void dgs_helper_power_on ();
void dgs_helper_power_off ();
int dgs_helper_power_status ();

#endif // _DGS_HELPER_H_
