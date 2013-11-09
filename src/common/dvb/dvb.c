#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#ifdef OLDDVBAPI
#include <ost/dmx.h>
#define dmx_pes_filter_params dmxPesFilterParams
#define dmx_sct_filter_params dmxSctFilterParams
#else
#include <linux/dvb/dmx.h>
#endif

#include "../../common.h"

#include "../core/log.h"

#include "dvb.h"

void dvb_read (dvb_t *settings, bool(*data_callback)(int, unsigned char*))
{
	struct pollfd PFD[256];
	int cycles, i, total_size;
	struct dmx_sct_filter_params params;
	dmx_source_t ssource;
	
	char first[settings->pids_count][settings->buffer_size];
	int first_length[settings->pids_count];
	bool first_ok[settings->pids_count];
	
	switch (settings->frontend)
	{
	case 1:
		ssource = DMX_SOURCE_FRONT1;
		break;
	case 2:
		ssource = DMX_SOURCE_FRONT2;
		break;
	case 3:
		ssource = DMX_SOURCE_FRONT3;
		break;
	case 4:
		ssource = DMX_SOURCE_FRONT4;
		break;
	case 5:
		ssource = DMX_SOURCE_FRONT5;
		break;
	case 6:
		ssource = DMX_SOURCE_FRONT6;
		break;
	case 7:
		ssource = DMX_SOURCE_FRONT7;
		break;
	default:
		ssource = DMX_SOURCE_FRONT0;
	}
	
	for (i = 0; i < settings->pids_count; i++)
	{
		PFD[i].fd = open (settings->demuxer, O_RDWR|O_NONBLOCK);
		PFD[i].events = POLLIN;
		PFD[i].revents = 0;
		
		memset (&params, 0, sizeof (params));
		params.pid = settings->pids[i];
		params.timeout = 5000;
		params.flags = DMX_CHECK_CRC|DMX_IMMEDIATE_START;
		params.filter.filter[0] = settings->filter;
		params.filter.mask[0] = settings->mask;
		
		if (ioctl(PFD[i].fd, DMX_SET_SOURCE, &ssource) < 0) {
			log_add ("ioctl DMX_SET_SOURCE failed");
		}
		
		if (ioctl (PFD[i].fd, DMX_SET_FILTER, &params) < 0)
			log_add ("ioctl DMX_SET_FILTER failed");
		
		first_length[i] = 0;
		first_ok[i] = false;
	}
	
	total_size = 0;
	cycles = 0;
	while ((cycles < MAX_OTV_LOOP_CYCLES) && (poll (PFD, settings->pids_count, 5000) > 0))
	{
		int k;
		bool ended = true;
		bool force_quit = false;
		for (i = 0; i < settings->pids_count; i++)
		{
			unsigned char buf[settings->buffer_size];	// 4K buffer size
			int size = 0;
			if (PFD[i].revents & POLLIN)
				size = read (PFD[i].fd, buf, sizeof (buf));
			
			if (size == -1) continue;
			if (first_ok[i]) continue;
			if (size < settings->min_length) continue;
			
			if (first_length[i] == 0)
			{
				first_length[i] = size;
				memcpy (first[i], buf, size);
			}
			else if (first_length[i] == size)
			{
				if (memcmp (buf, first[i], size) == 0) first_ok[i] = true;
			}
			
			total_size += size;
			//data_callback (size, buf);
			force_quit = !data_callback (size, buf);
		}
		
		for (k = 0; k < settings->pids_count; k++)
			ended &= first_ok[k];
		
		if (ended || force_quit) break;
		
		cycles++;
	}
	
	if (cycles == MAX_OTV_LOOP_CYCLES) log_add ("Maximum loop exceded");
	
	for (i = 0; i < settings->pids_count; i++)	// close filters
	{
		if (ioctl (PFD[i].fd, DMX_STOP) < 0)
			log_add ("Error stopping filter");
		close (PFD[i].fd);
	}
}
