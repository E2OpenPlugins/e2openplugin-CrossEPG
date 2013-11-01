#ifndef _DVB_H_
#define _DVB_H_

typedef struct buffer_s
{
	unsigned short	size;
	unsigned char	*data;
} buffer_t;

typedef struct dvb_s
{
	int				*pids;
	unsigned int	pids_count;
	char			*demuxer;
	int				frontend;
	unsigned int	min_length;
	unsigned int	buffer_size;
	unsigned char 	filter;
	unsigned char	mask;
} dvb_t;

void dvb_read (dvb_t *settings, bool(*data_callback)(int, unsigned char*));
//void dvb_read (int *pids, int pids_count);
//bool log_open (char *file, char *plugin_name);
//void log_close ();
//void log_add (char *message, ...);

#endif // _DVB_H_
