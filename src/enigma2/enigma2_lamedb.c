#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#include "../common.h"

#include "enigma2_lamedb.h"

#define MAX_CHANNELS 65535	// is it enought?

static int nids[MAX_CHANNELS];
static int tsids[MAX_CHANNELS];
static int sids[MAX_CHANNELS];
static int namespaces[MAX_CHANNELS];
static int channels_count;

bool enigma2_lamedb_read (char *filename)
{
	char line[512];
	
	FILE *fd = fopen (filename, "r");
	if (!fd) return false;

	channels_count = 0;
	while (fgets (line, sizeof(line), fd)) 
	{
		int nid, tsid, sid, namespace, trash2, trash3;
		
		if (sscanf (line, "%x:%x:%x:%x:%x:%x\n", &sid, &namespace, &tsid, &nid, &trash2, &trash3) == 6)
		{
			nids[channels_count] = nid;
			tsids[channels_count] = tsid;
			sids[channels_count] = sid;
			namespaces[channels_count] = namespace;
			channels_count++;
		}
	}

	fclose (fd);
	return true;
}

bool enigma2_lamedb_exist (int nid, int tsid, int sid)
{
	int i;
	for (i=0; i < channels_count; i++)
		if ((nids[i] == nid) && (tsids[i] == tsid) && (sids[i] == sid)) return true;

	return false;
}

int enigma2_lamedb_get_namespace (int nid, int tsid, int sid)
{
	int i;
	for (i=0; i < channels_count; i++)
		if ((nids[i] == nid) && (tsids[i] == tsid) && (sids[i] == sid)) return namespaces[i];

	return -1;
}
