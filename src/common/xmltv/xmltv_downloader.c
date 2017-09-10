#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>
#include "zlib.h"

#include "../../common.h"

#include "../core/log.h"
#include "../aliases/aliases.h"
#include "../epgdb/epgdb.h"
#include "../net/http.h"
#include "../gzip/gzip.h"

#include "xmltv_channels.h"
#include "xmltv_parser.h"
#include "xmltv_downloader.h"

static char _url[256];
//static volatile bool _stop = false;

#define BUFLEN      16384

static bool xmltv_downloader_extension_check (char *filename, char *extension)
{
	if (strlen (filename) <= strlen (extension) + 1) return false;
	if (memcmp (filename + (strlen (filename) - strlen (extension)), extension, strlen (extension)) != 0) return false;
	if (filename[strlen (filename) - strlen (extension) - 1] != '.') return false;
	return true;
}

bool xmltv_downloader_channels (char *url, char *dbroot, void(*progress_callback)(int, int), void(*event_callback)(int, char*), volatile bool *stop)
{
	bool ret = false;
	char host[256];
	char port[6];
	char page[256];
	
	char *tmp = url;
	int pos = 0;
	
	strcpy (_url, url);
	
	if (event_callback) event_callback(2, _url);	// download message
	if (event_callback) event_callback(0, NULL);	// turn on progress bar

	memset (host, 0, sizeof (host));
	memset (port, 0, sizeof (port));
	memset (page, 0, sizeof (page));
	
	if (strlen (tmp) < 7) return false;
	if (memcmp (tmp, "http://", 7) != 0) return false;
	tmp += 7;
	while ((tmp[0+pos] != '\0') && (tmp[0+pos] != ':') && (tmp[0+pos] != '/')) pos++;
	if (pos > (sizeof (host) - 1)) return false;
	memcpy (host, tmp, pos);
	tmp += pos;
	if (tmp[0] == ':')
	{
		tmp++;
		pos = 0;
		while ((tmp[0+pos] != '\0') && (tmp[0+pos] != '/')) pos++;
		if (pos > (sizeof (port) - 1)) return false;
		memcpy (port, tmp, pos);
		tmp += pos;
	}
	else
		sprintf (port, "80");

	tmp++;
	pos = 0;
	while ((tmp[0+pos] != '\0') && (tmp[0+pos] != '\n')) pos++;
	if (pos > (sizeof (page) - 1)) return false;
	memcpy (page, tmp, pos);

	char sfn[256];
	int fd = -1;
	sprintf (sfn, "%s/crossepg.tmp.XXXXXX", dbroot);
	if ((fd = mkstemp (sfn)) == -1)
	{
		if (event_callback) event_callback(1, NULL);	// turn off progress bar
		log_add ("Cannot get temp file (%s)", sfn);
		return false;
	}
	char tmp_url[256];
	sprintf (tmp_url, "http://%s/%s", host, page);
	if (!http_get (host, page, atoi (port), fd, progress_callback, stop))
	{
		if (event_callback) event_callback(1, NULL);	// turn off progress bar
		log_add ("Error downloading file %s", tmp_url);
		unlink (sfn);
		return false;
	}
	
	if (event_callback) event_callback(1, NULL);	// turn off progress bar

	if (*stop)
	{
		unlink (sfn);
		return false;
	}

	if (xmltv_downloader_extension_check (page, "gz"))
	{
		int fd2 = -1;
		char sfn2[256];
		sprintf (sfn2, "%s/crossepg.tmp.XXXXXX", dbroot);
		if ((fd2 = mkstemp (sfn2)) == -1) log_add ("Cannot get temp file");
		else
		{
			if (event_callback) event_callback(3, NULL);	// deflating message
			log_add ("Deflating %s", page);
			FILE *dest = fdopen (fd2, "w");
			if (!gzip_inf (sfn, dest)) log_add ("Error deflating file");
			else log_add ("File deflated");
			fclose (dest);
			close (fd2);
			if (event_callback) event_callback(4, NULL);	// reading message
			ret = xmltv_channels_load (sfn2);
			unlink (sfn2);
		}	
	}
	else if (xmltv_downloader_extension_check (page, "xz"))
	{
		char cmdxz[256];
		int fd2 = -1;
		char sfn2[256];
		char sfn3[256];
		sprintf (sfn2, "%s/crossepg.tmp.XXXXXX", dbroot);
		sprintf (sfn3, "%s/cross.xz", dbroot);
		if ((fd2 = mkstemp (sfn2)) == -1) log_add ("Cannot get temp file");
		else
		{
			sprintf (cmdxz, "cp %s %s", sfn, sfn3);
			system(cmdxz);
			sprintf (cmdxz, "xz -d %s", sfn3);
			system(cmdxz);			
			if (event_callback) event_callback(3, NULL);	// deflating message
			log_add ("Deflating %s", page);
			close (fd2);
			sprintf (sfn3, "%s/cross", dbroot);
			sprintf (cmdxz, "mv -f %s %s", sfn3, sfn2);
			system(cmdxz);
			if (event_callback) event_callback(4, NULL);	// reading message
			ret = xmltv_channels_load (sfn2);
			unlink (sfn2);
		}
	}
	else
	{
		if (event_callback) event_callback(4, NULL);	// reading message
		ret = xmltv_channels_load (sfn);
	}
	
	unlink (sfn);

	return ret;
}

bool xmltv_downloader_events (char *url, char *dbroot, void(*progress_callback)(int, int), void(*event_callback)(int, char*), volatile bool *stop)
{
	bool ret = false;
	char host[256];
	char port[6];
	char page[256];
	
	char *tmp = url;
	int pos = 0;
	
	strcpy (_url, url);
	
	if (event_callback) event_callback(5, _url);	// download message
	if (event_callback) event_callback(0, NULL);	// turn on progress bar

	memset (host, 0, sizeof (host));
	memset (port, 0, sizeof (port));
	memset (page, 0, sizeof (page));
	
	if (strlen (tmp) < 7) return false;
	if (memcmp (tmp, "http://", 7) != 0) return false;
	tmp += 7;
	while ((tmp[0+pos] != '\0') && (tmp[0+pos] != ':') && (tmp[0+pos] != '/')) pos++;
	if (pos > (sizeof (host) - 1)) return false;
	memcpy (host, tmp, pos);
	tmp += pos;
	if (tmp[0] == ':')
	{
		tmp++;
		pos = 0;
		while ((tmp[0+pos] != '\0') && (tmp[0+pos] != '/')) pos++;
		if (pos > (sizeof (port) - 1)) return false;
		memcpy (port, tmp, pos);
		tmp += pos;
	}
	else
		sprintf (port, "80");

	tmp++;
	pos = 0;
	while ((tmp[0+pos] != '\0') && (tmp[0+pos] != '\n')) pos++;
	if (pos > (sizeof (page) - 1)) return false;
	memcpy (page, tmp, pos);

	char sfn[256];
	int fd = -1;
	sprintf (sfn, "%s/crossepg.tmp.XXXXXX", dbroot);
	if ((fd = mkstemp (sfn)) == -1)
	{
		log_add ("Cannot get temp file (%s)", sfn);
		if (event_callback) event_callback(1, NULL);	// turn off progress bar
		return false;
	}
	char tmp_url[256];
	sprintf (tmp_url, "http://%s/%s", host, page);
	if (!http_get (host, page, atoi (port), fd, progress_callback, stop))
	{
		log_add ("Error downloading file %s", tmp_url);
		if (event_callback) event_callback(1, NULL);	// turn off progress bar
		return false;
	}

	if (event_callback) event_callback(1, NULL);	// turn off progress bar

	if (*stop)
	{
		unlink (sfn);
		return false;
	}

	if (xmltv_downloader_extension_check (page, "gz"))
	{
		int fd2 = -1;
		char sfn2[256];
		sprintf (sfn2, "%s/crossepg.tmp.XXXXXX", dbroot);
		if ((fd2 = mkstemp (sfn2)) == -1) log_add ("Cannot get temp file");
		else
		{
			if (event_callback) event_callback(6, NULL);	// deflating message
			log_add ("Deflating %s", page);
			FILE *dest = fdopen (fd2, "w");
			if (!gzip_inf (sfn, dest)) log_add ("Error deflating file");
			else log_add ("File deflated");
			fclose (dest);
			close (fd2);
			if (event_callback) event_callback(7, NULL);	// parsing events
			if (event_callback) event_callback(0, NULL);	// turn on progress bar
			ret = xmltv_parser_import (sfn2, progress_callback, stop);
			if (event_callback) event_callback(1, NULL);	// turn off progress bar
			unlink (sfn2);
		}
	}
	
	else if (xmltv_downloader_extension_check (page, "xz"))
	{
		char cmdxz[256];
		int fd2 = -1;
		char sfn2[256];
		char sfn3[256];
		sprintf (sfn2, "%s/crossepg.tmp.XXXXXX", dbroot);
		sprintf (sfn3, "%s/cross.xz", dbroot);
		if ((fd2 = mkstemp (sfn2)) == -1) log_add ("Cannot get temp file");
		else
		{
			sprintf (cmdxz, "cp %s %s", sfn, sfn3);
			system(cmdxz);
			sprintf (cmdxz, "xz -d %s", sfn3);
			system(cmdxz);			
			if (event_callback) event_callback(6, NULL);	// deflating message
			log_add ("Deflating %s", page);
			close (fd2);
			sprintf (sfn3, "%s/cross", dbroot);
			sprintf (cmdxz, "mv -f %s %s", sfn3, sfn2);
			system(cmdxz);
			if (event_callback) event_callback(7, NULL);	// parsing events
			if (event_callback) event_callback(0, NULL);	// turn on progress bar
			ret = xmltv_parser_import (sfn2, progress_callback, stop);
			if (event_callback) event_callback(1, NULL);	// turn off progress bar
			unlink (sfn2);
		}
		
	}
	else
	{
		if (event_callback) event_callback(7, NULL);	// parsing events
		if (event_callback) event_callback(0, NULL);	// turn on progress bar
		ret = xmltv_parser_import (sfn, progress_callback, stop);
		if (event_callback) event_callback(1, NULL);	// turn off progress bar
	}
	
	unlink (sfn);

	return ret;
}
