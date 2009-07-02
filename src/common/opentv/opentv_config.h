#ifndef _OPENTV_CONFIG_H_
#define _OPENTV_CONFIG_H_

int  *opentv_config_get_channels_pids ();
int  *opentv_config_get_titles_pids ();
int  *opentv_config_get_summaries_pids ();
int  *opentv_config_get_channels_types ();
int  opentv_config_get_channels_pids_count ();
int  opentv_config_get_titles_pids_count ();
int  opentv_config_get_summaries_pids_count ();
int  opentv_config_get_channels_types_count ();
int  opentv_config_get_nid ();
int  opentv_config_get_tsid ();
int  opentv_config_get_sid ();
bool opentv_config_read (char *read);

#endif // _OPENTV_CONFIG_H_
