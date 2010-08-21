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
#ifndef E2
#include <sqlite3.h>
#endif
#include "zlib.h"

#include "../../common.h"

#include "../core/log.h"
#include "../core/config.h"
#ifndef E2
#include "../../dgs/dgs.h"
#endif
#include "../aliases/aliases.h"
#include "../epgdb/epgdb.h"
#include "../net/http.h"

#include "xmltv_channels.h"
#include "xmltv_parser.h"
#include "xmltv_downloader.h"

static char _url[256];
static volatile bool _stop = false;

#define BUFLEN      16384

static bool xmltv_downloader_gzip (char *source, FILE *dest)
{
	char buf[BUFLEN];
	int len, err;
	gzFile in = gzopen (source, "rb");
	for (;;)
	{
		len = gzread(in, buf, sizeof (buf));
		if (len < 0)
		{
			log_add ("%s", gzerror (in, &err));
			gzclose (in);
			return false;
		}
		if (len == 0) break;
		fwrite (buf, 1, len, dest);
	}
	gzclose (in);
	return true;
}

static bool xmltv_downloader_extension_check (char *filename, char *extension)
{
	if (strlen (filename) <= strlen (extension) + 1) return false;
	if (memcmp (filename + (strlen (filename) - strlen (extension)), extension, strlen (extension)) != 0) return false;
	if (filename[strlen (filename) - strlen (extension) - 1] != '.') return false;
	return true;
}

bool xmltv_downloader_channels (char *url, char *dbroot)
{
	bool ret = false;
	char host[256];
	char port[6];
	char page[256];
	
	char *tmp = url;
	int pos = 0;
	
	strcpy (_url, url);
	
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
		return false;
	}
	char tmp_url[256];
	sprintf (tmp_url, "http://%s/%s", host, page);
	if (!http_get (host, page, atoi (port), fd, NULL, &_stop))
	{
		log_add ("Error downloading file %s", tmp_url);
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
			log_add ("Deflating %s", page);
			FILE *dest = fdopen (fd2, "w");
			if (!xmltv_downloader_gzip (sfn, dest)) log_add ("Error deflating file");
			else log_add ("File deflated");
			fclose (dest);
			close (fd2);
			ret = xmltv_channels_load (sfn2);
			unlink (sfn2);
		}
	}
	else
		ret = xmltv_channels_load (sfn);
	
	unlink (sfn);

	return ret;
}

bool xmltv_downloader_events (char *url, char *dbroot)
{
	bool ret = false;
	char host[256];
	char port[6];
	char page[256];
	
	char *tmp = url;
	int pos = 0;
	
	strcpy (_url, url);
	
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
		return false;
	}
	char tmp_url[256];
	sprintf (tmp_url, "http://%s/%s", host, page);
	if (!http_get (host, page, atoi (port), fd, NULL, &_stop))
	{
		log_add ("Error downloading file %s", tmp_url);
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
			log_add ("Deflating %s", page);
			FILE *dest = fdopen (fd2, "w");
			if (!xmltv_downloader_gzip (sfn, dest)) log_add ("Error deflating file");
			else log_add ("File deflated");
			fclose (dest);
			close (fd2);
			ret = xmltv_parser_import (sfn2);
			unlink (sfn2);
		}
	}
	else
		ret = xmltv_parser_import (sfn);
	
	unlink (sfn);

	return ret;
}
