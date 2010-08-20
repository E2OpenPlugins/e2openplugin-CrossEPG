#ifndef _XMLTV_CHANNELS_H_
#define _XMLTV_CHANNELS_H_

typedef struct xmltv_channel_s
{
	unsigned short int	nid;
	unsigned short int	tsid;
	unsigned short int	sid;
	char *id;
	struct xmltv_channel_s *prev;
	struct xmltv_channel_s *next;
} xmltv_channel_t;

void xmltv_channels_init ();
void xmltv_channels_cleanup ();
bool xmltv_channels_load (char *filename);
xmltv_channel_t* xmltv_channels_get_by_id (char *id, xmltv_channel_t* last);

#endif // _XMLTV_CHANNELS_H_
