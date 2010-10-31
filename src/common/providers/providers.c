#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "../../common.h"

#include "../core/log.h"

#include "providers.h"

static int protocol = 0; // 0 unknown, 1 opentv, 2 xmltv
static int channels_pids[64];
static int channels_pids_count = 0;
static int titles_pids[64];
static int titles_pids_count = 0;
static int summaries_pids[64];
static int summaries_pids_count = 0;
static int channels_types[64];
static int channels_types_count = 0;
static int nid = 0;
static int tsid = 0;
static int sid = 0;
static char channels[256];
static char url[256];
static char xepgdb_headers_url[256];
static char xepgdb_descriptors_url[256];
static char preferred_language[4];

int  *providers_get_channels_pids			()	{ return channels_pids;			}
int  *providers_get_titles_pids				()	{ return titles_pids;			}
int  *providers_get_summaries_pids			()	{ return summaries_pids;		}
int  *providers_get_channels_types			()	{ return channels_types;		}
int  providers_get_channels_pids_count		()	{ return channels_pids_count;	}
int  providers_get_titles_pids_count		()	{ return titles_pids_count;		}
int  providers_get_summaries_pids_count		()	{ return summaries_pids_count;	}
int  providers_get_channels_types_count		()	{ return channels_types_count;	}
int  providers_get_nid						()	{ return nid;					}
int  providers_get_tsid						()	{ return tsid;					}
int  providers_get_sid						()	{ return sid;					}
int  providers_get_protocol					()	{ return protocol;				}
char *providers_get_xmltv_channels			()	{ return channels;				}
char *providers_get_xmltv_url				()	{ return url;					}
char *providers_get_xmltv_plang				()	{ return preferred_language;	}
char *providers_get_xepgdb_headers_url		()	{ return xepgdb_headers_url;	}
char *providers_get_xepgdb_descriptors_url	()	{ return xepgdb_descriptors_url;}

static char *providers_trim_spaces (char *text)
{
	char *tmp = text;
	while (tmp[0] == ' ') tmp++;
	while (strlen (tmp) > 1)
		if (tmp[strlen (tmp) - 1] == ' ') tmp[strlen (tmp) - 1] = '\0';
		else break;
	
	if (tmp[0] == ' ') tmp[0] = '\0';
	return tmp;
}

bool providers_read (char *read)
{
	FILE *fd = NULL;
	char line[512];
	char key[256];
	char value[256];
	
	channels_pids_count = 0;
	titles_pids_count = 0;
	summaries_pids_count = 0;
	channels_types_count = 0;
	protocol = 0;
	
	strcpy (channels, "");
	strcpy (url, "");
	strcpy (preferred_language, "eng");
	strcpy (xepgdb_headers_url, "");
	strcpy (xepgdb_descriptors_url, "");
	
	fd = fopen (read, "r");
	if (!fd) 
		return false;
	
	while (fgets (line, sizeof(line), fd)) 
	{
		char *tmp_key, *tmp_value;
		
		memset (key, 0, sizeof (key));
		memset (value, 0, sizeof (value));
		
		if (sscanf (line, "%[^#=]=%s\n", key, value) != 2)
			continue;

		tmp_key = providers_trim_spaces (key);
		tmp_value = providers_trim_spaces (value);

		if (strcmp ("protocol", tmp_key) == 0)
		{
			if (strcmp ("opentv", tmp_value) == 0)
				protocol = 1;
			else if (strcmp ("xmltv", tmp_value) == 0)
				protocol = 2;
			else if (strcmp ("xepgdb", tmp_value) == 0)
				protocol = 3;
			else if (strcmp ("script", tmp_value) == 0)
				protocol = 4;
		}
		else if (strcmp ("channels_pids", tmp_key) == 0)
		{
			char* tmp = strtok (tmp_value, "|");
			while ((tmp != NULL) && (channels_pids_count < 64))
			{
				channels_pids[channels_pids_count] = atoi (tmp);
				tmp = strtok (NULL, "|");
				channels_pids_count++;
			}
		}
		else if (strcmp ("titles_pids", tmp_key) == 0)
		{
			char* tmp = strtok (tmp_value, "|");
			while ((tmp != NULL) && (titles_pids_count < 64))
			{
				titles_pids[titles_pids_count] = atoi (tmp);
				tmp = strtok (NULL, "|");
				titles_pids_count++;
			}
		}
		else if (strcmp ("summaries_pids", tmp_key) == 0)
		{
			char* tmp = strtok (tmp_value, "|");
			while ((tmp != NULL) && (summaries_pids_count < 64))
			{
				summaries_pids[summaries_pids_count] = atoi (tmp);
				tmp = strtok (NULL, "|");
				summaries_pids_count++;
			}
		}
		else if (strcmp ("channels_types", tmp_key) == 0)
		{
			char* tmp = strtok (tmp_value, "|");
			while ((tmp != NULL) && (channels_types_count < 64))
			{
				channels_types[channels_types_count] = atoi (tmp);
				tmp = strtok (NULL, "|");
				channels_types_count++;
			}
		}
		else if (strcmp ("nid", tmp_key) == 0)
			nid = atoi (tmp_value);
		else if (strcmp ("tsid", tmp_key) == 0)
			tsid = atoi (tmp_value);
		else if (strcmp ("sid", tmp_key) == 0)
			sid = atoi (tmp_value);
		else if (strcmp ("channels", tmp_key) == 0)
			strcpy (channels, tmp_value);
		else if (strcmp ("url", tmp_key) == 0)
			strcpy (url, tmp_value);
		else if (strcmp ("preferred_language", tmp_key) == 0)
		{
			if (strlen (tmp_value) == 3)
				strcpy (preferred_language, tmp_value);
		}
		else if (strcmp ("headers_url", tmp_key) == 0)
			strcpy (xepgdb_headers_url, tmp_value);
		else if (strcmp ("descriptors_url", tmp_key) == 0)
			strcpy (xepgdb_descriptors_url, tmp_value);
	}
	
	fclose (fd);
	
	return true;
}
