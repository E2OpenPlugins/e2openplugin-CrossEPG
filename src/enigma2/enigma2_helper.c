#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "../common.h"

#include "../common/core/log.h"
#include "../common/net/http.h"

#include "enigma2_helper.h"

#define SERVICEID_START_TAG "<e2servicereference>"
#define SERVICEID_STOP_TAG "</e2servicereference>"

bool enigma2_helper_get_channel (int *nid, int *tsid, int *sid)
{
	char sfn[256];
	int temp_file = -1;
	bool ret = false;
	sprintf (sfn, "/tmp/crossepg.tmp.XXXXXX");
    if ((temp_file = mkstemp (sfn)) == -1)
	{
		log_add ("Cannot get temp file");
		return false;
	}
	
	ret = http_get ("127.0.0.1", "web/subservices", 80, temp_file, NULL);
	close (temp_file);

	if (ret)
	{
		ret = false;
		char line[256];
		FILE *fd;
		fd = fopen (sfn, "r");
		
		if (fd != NULL)
		{
			while (fgets (line, sizeof(line), fd)) 
			{
				char *start = strstr (line, SERVICEID_START_TAG);
				if (start != NULL)
				{
					char *end = strstr (line, SERVICEID_STOP_TAG);
					if (end != NULL)
					{
						int trash1, trash2, trash3, trash4, trash5, trash6, trash7;
						end[0] = '\0';
						start += strlen (SERVICEID_START_TAG);
						
						// 1:0:1:E33:16A8:FBFF:820000:0:0:0:
						if (sscanf (start, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x", &trash1, &trash2, &trash3, sid, tsid, nid, &trash4, &trash5, &trash6, &trash7) == 10)
						{
							ret = true;
							break;
						}
					}
				}
			}
		}
	}
	else
		log_add ("Error reading current channel");

	unlink (sfn);
	
	return ret;
}

bool enigma2_helper_change_channel (int nid, int tsid, int sid, bool with_check)
{
	char sfn[256];
	char page[256];
	int temp_file = -1, i;
	bool ret = false;
	
	sprintf (sfn, "/tmp/crossepg.tmp.XXXXXX");
    if ((temp_file = mkstemp (sfn)) == -1)
	{
		log_add ("Cannot get temp file");
		return false;
	}
	sprintf (page, "web/zap?sRef=1:0:1:%X:%X:%X:820000:0:0:0:", sid, tsid, nid);
	ret = http_get ("127.0.0.1", page, 80, temp_file, NULL);
	close (temp_file);
	unlink (sfn);
	
	if (!ret) return false;
	
	if (with_check)
	{
		ret = false;

		for (i=0; i<10; i++)
		{
			int tmp_nid, tmp_tsid, tmp_sid;
			usleep (1000000);
			if (enigma2_helper_get_channel (&tmp_nid, &tmp_tsid, &tmp_sid))
			{
				if ((tmp_nid == nid) && (tmp_tsid == tsid) && (tmp_sid == sid))
				{
					ret = true;
					break;
				}
			}
		}
	}
	
	return ret;
}

