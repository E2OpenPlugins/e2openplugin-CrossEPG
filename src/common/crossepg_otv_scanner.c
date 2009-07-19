#include <stdio.h>
#include <string.h>
#include <linux/dvb/dmx.h>
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

#include "../common.h"

#include "core/log.h"

#undef MAX_OTV_LOOP_CYCLES
#define MAX_OTV_LOOP_CYCLES			200

void scan_opentv (char *demuxer, unsigned short int start_pid, unsigned short int pids_count)
{
	struct pollfd PFD[pids_count];
	int cycles, i, filters_started, filters_skipped;
	struct dmx_sct_filter_params params;
	int titles[pids_count];
	int summaries[pids_count];
	
	filters_started = 0;
	filters_skipped = 0;
	
	for (i = 0; i < pids_count; i++)		// load filters
	{
		PFD[i].fd = open (demuxer, O_RDWR|O_NONBLOCK);
		PFD[i].events = POLLIN;
		PFD[i].revents = 0;
		
		memset (&params, 0, sizeof (params));
		params.pid = start_pid + i;
		params.timeout = 5000;
		params.flags = DMX_CHECK_CRC|DMX_IMMEDIATE_START;
		params.filter.filter[0] = 0x00;
		params.filter.mask[0] = 0x00;
		
		if (ioctl (PFD[i].fd, DMX_SET_FILTER, &params ) < 0 )
			filters_skipped++;
		else
			filters_started++;
		
		titles[i] = 0;
		summaries[i] = 0;
	}

	if (filters_skipped > 0)
		log_add ("Skipped %d filters", filters_skipped);
	
	cycles = 0;
	while ((cycles < MAX_OTV_LOOP_CYCLES) && (poll (PFD, pids_count, 5000) > 0))
	{
		for (i = 0; i < pids_count; i++)
		{
			unsigned char buf[4*1024];	// 4K buffer size
			int size = 0;
			if (PFD[i].revents & POLLIN)
				size = read (PFD[i].fd, buf, sizeof (buf));
			
			if (size >= 20)
			{
				if ((buf[0] == 0xa0) || (buf[0] == 0xa1) || (buf[0] == 0xa2) || (buf[0] == 0xa3)) titles[i]++;
				if (buf[14] == 0xb9) summaries[i]++;
			}
		}
		
		cycles++;
	}

	for (i = 0; i < pids_count; i++)	// close filters
	{
		if (titles[i] > 5)
			log_add ("Possible titles on pid 0x%x", i + start_pid);
		if (summaries[i] > 5)
			log_add ("Possible summaries on pid 0x%x", i + start_pid);
		ioctl (PFD[i].fd, DMX_STOP);
		close (PFD[i].fd);
	}
	
}

int main (int argc, char **argv)
{
	int c, i;
	unsigned short int start_pid = 0;
	unsigned short int stop_pid = 255;
	int pids_count;
	
	char *demuxer = "/dev/dvb/adapter0/demux0";
    
	opterr = 0;
  
	log_open (NULL, "CrossEPG OpenTV data scanner");
  
	while ((c = getopt (argc, argv, "x:")) != -1)
	{
		switch (c)
		{
			case 'x':
				demuxer = optarg;
				break;
			case '?':
				printf ("Usage:\n");
				printf ("  ./crossepg_otv_scanner [options]\n");
				printf ("Options:\n");
				printf ("  -x demuxer    dvb demuxer\n");
				printf ("  -h            show this help\n");
				return 0;
		}
	}
	
	pids_count = (stop_pid - start_pid) + 1;
	log_add ("Start pid: %x", start_pid);
	log_add ("Stop pid: %x", stop_pid);
	log_add ("Pids count: %d", pids_count);
	log_add ("Started OpenTV Scan");
	
	for (i=0; i<(pids_count-(pids_count%16)-1); i=i+16)
	{
		log_add ("Scanning range 0x%x <=> 0x%x", i, i+15);
		scan_opentv (demuxer, i, 16);
	}

	if (i<(pids_count-1))
	{
		log_add ("Scanning range 0x%x <=> 0x%x", i, pids_count - 1);
		scan_opentv (demuxer, i, pids_count - i);
	}

	log_add ("Ended OpenTV scan");
	
	memory_stats ();
	log_close ();
	return 0;
}
