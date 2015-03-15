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

#include "../../common.h"

#include "../core/log.h"
#include "../aliases/aliases.h"
#include "../epgdb/epgdb.h"
#include "../epgdb/epgdb_channels.h"
#include "../epgdb/epgdb_titles.h"
#include "../net/http.h"
#include "../gzip/gzip.h"

#include "dbmerge.h"

#define MAGIC_HEADERS	"_xEPG_HEADERS"

typedef struct epgdb_channel_header_s
{
	uint16_t	nid;
	uint16_t	tsid;
	uint16_t	sid;
} epgdb_channel_header_t;

typedef struct epgdb_title_header_s
{
	uint16_t	event_id;
	uint16_t	mjd;
	time_t		start_time;
	uint16_t	length;
	uint8_t		genre_id;
	uint8_t		flags;
	uint32_t	description_crc;
	uint32_t	description_seek;
	uint32_t	long_description_crc;
	uint32_t	long_description_seek;
	uint16_t	description_length;
	uint16_t	long_description_length;
	uint8_t		iso_639_1;
	uint8_t		iso_639_2;
	uint8_t		iso_639_3;
	uint8_t		revision;

} epgdb_title_header_t;

static bool dbmerge_download (char *url, int fd, void(*progress_callback)(int, int), void(*event_callback)(int, char*), volatile bool *stop)
{
	char host[256];
	char port[6];
	char page[256];

	char *tmp = url;
	int pos = 0;

	if (event_callback) event_callback(8, url);	// download message
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

	char tmp_url[256];
	sprintf (tmp_url, "http://%s/%s", host, page);
	if (!http_get (host, page, atoi (port), fd, progress_callback, stop))
	{
		if (event_callback) event_callback(1, NULL);	// turn off progress bar
		log_add ("Error downloading file %s", tmp_url);
		return false;
	}

	if (event_callback) event_callback(1, NULL);	// turn off progress bar

	if (*stop)
		return false;

	return true;
}

static bool dbmerge_extension_check (char *filename, char *extension)
{
	if (strlen (filename) <= strlen (extension) + 1) return false;
	if (memcmp (filename + (strlen (filename) - strlen (extension)), extension, strlen (extension)) != 0) return false;
	if (filename[strlen (filename) - strlen (extension) - 1] != '.') return false;
	return true;
}

static char *dbmerge_read_description (FILE *fd_d, epgdb_title_t *title)
{
	char *ret = _malloc (title->description_length + 1);
	memset (ret, '\0', title->description_length + 1);

	fseek (fd_d, title->description_seek, SEEK_SET);
	fread (ret, title->description_length, 1, fd_d);
	return ret;
}

static char *dbmerge_read_long_description (FILE *fd_d, epgdb_title_t *title)
{
	char *ret = _malloc (title->long_description_length + 1);
	memset (ret, '\0', title->long_description_length + 1);

	fseek (fd_d, title->long_description_seek, SEEK_SET);
	fread (ret, title->long_description_length, 1, fd_d);
	return ret;
}

bool dbmerge_merge (FILE *fd_h, FILE *fd_d, void(*progress_callback)(int, int))
{
	char tmp[256];
	unsigned char revision;
	int channels_count, i, j, aliases_groups_count, indexes_count;
	time_t now = time (NULL);

	/* read headers */
	fread (tmp, strlen (MAGIC_HEADERS), 1, fd_h);
	if (memcmp (tmp, MAGIC_HEADERS, strlen (MAGIC_HEADERS)) != 0)
	{
		log_add ("Bad magic header");
		return false;
	}
	fread (&revision, sizeof (unsigned char), 1, fd_h);
	if (revision != DB_REVISION)
	{
		log_add ("Bad db revision");
		return false;
	}

	fseek (fd_h, 22, SEEK_SET);

	fread (&channels_count, sizeof (int), 1, fd_h);
	for (i=0; i<channels_count; i++)
	{
		int titles_count;
		epgdb_channel_t *tmp;
		epgdb_channel_t *channel, *channel_tmp;

		channel_tmp = _malloc (sizeof (epgdb_channel_t));
		fread (channel_tmp, sizeof (epgdb_channel_header_t), 1, fd_h);
		channel = epgdb_channels_add (channel_tmp->nid, channel_tmp->tsid, channel_tmp->sid);
		_free (channel_tmp);

		fread (&titles_count, sizeof (int), 1, fd_h);

		for (j=0; j<titles_count; j++)
		{
			char *desc, *ldesc;
			epgdb_title_t *title = _malloc (sizeof (epgdb_title_t));
			fread (title, sizeof (epgdb_title_header_t), 1, fd_h);

			desc = dbmerge_read_description (fd_d, title);
			ldesc = dbmerge_read_long_description (fd_d, title);

			title = epgdb_titles_add (channel, title);

			epgdb_titles_set_description(title, desc);
			epgdb_titles_set_long_description(title, ldesc);

			_free (desc);
			_free (ldesc);
		}

		if (progress_callback)
			progress_callback (i, channels_count);
	}


	return true;
}

bool dbmerge_downloader (char *hashes_url, char *descriptors_url, char *dbroot, void(*progress_callback)(int, int), void(*event_callback)(int, char*), volatile bool *stop)
{
	char sfn[256], sfn2[256];
	int hashes_fd = -1, descriptors_fd = -1;
	FILE *fd_h, *fd_d;
	bool ret = false;

	sprintf (sfn, "%s/crossepg.tmp.XXXXXX", dbroot);
	sprintf (sfn2, "%s/crossepg.tmp.XXXXXX", dbroot);

	if ((hashes_fd = mkstemp (sfn)) == -1)
	{
		log_add ("Cannot get temp file (%s)", sfn);
		return false;
	}

	if ((descriptors_fd = mkstemp (sfn2)) == -1)
	{
		unlink (sfn);
		log_add ("Cannot get temp file (%s)", sfn2);
		return false;
	}

	if (!dbmerge_download(hashes_url, hashes_fd, progress_callback, event_callback, stop))
		goto error;

	if (!dbmerge_download(descriptors_url, descriptors_fd, progress_callback, event_callback, stop))
		goto error;

	if (dbmerge_extension_check (hashes_url, "gz"))
	{
		int fdtmp = -1;
		char sfntmp[256];
		sprintf (sfntmp, "%s/crossepg.tmp.XXXXXX", dbroot);
		if ((fdtmp = mkstemp (sfntmp)) == -1)
		{
			log_add ("Cannot get temp file");
			goto error;
		}
		else
		{
			if (event_callback) event_callback(10, hashes_url);	// deflating message
			log_add ("Deflating %s", sfn);
			FILE *dest = fdopen (fdtmp, "w");
			if (!gzip_inf (sfn, dest)) log_add ("Error deflating file");
			else log_add ("File deflated");
			fclose (dest);
			close (fdtmp);
			unlink (sfn);
			strcpy(sfn, sfntmp);
		}
	}

	if (dbmerge_extension_check (descriptors_url, "gz"))
	{
		int fdtmp = -1;
		char sfntmp[256];
		sprintf (sfntmp, "%s/crossepg.tmp.XXXXXX", dbroot);
		if ((fdtmp = mkstemp (sfntmp)) == -1)
		{
			log_add ("Cannot get temp file");
			goto error;
		}
		else
		{
			if (event_callback) event_callback(10, descriptors_url);	// deflating message
			log_add ("Deflating %s", sfn2);
			FILE *dest = fdopen (fdtmp, "w");
			if (!gzip_inf (sfn2, dest)) log_add ("Error deflating file");
			else log_add ("File deflated");
			fclose (dest);
			close (fdtmp);
			unlink (sfn2);
			strcpy(sfn2, sfntmp);
		}
	}

	fd_h = fopen (sfn, "r");
	if (fd_h == NULL)
	{
		log_add ("Cannot open %s", sfn);
		goto error;
	}

	fd_d = fopen (sfn2, "r");
	if (fd_d == NULL)
	{
		log_add ("Cannot open %s", sfn2);
		fclose (fd_h);
		goto error;
	}

	if (event_callback) event_callback(9, hashes_url);	// parsing xepgdb
	if (event_callback) event_callback(0, NULL);	// turn on progress bar
	ret = dbmerge_merge(fd_h, fd_d, progress_callback);
	if (event_callback) event_callback(1, NULL);	// turn off progress bar

	fclose(fd_h);
	fclose(fd_d);

error:
	unlink (sfn);
	unlink (sfn2);
	return ret;
}

bool dbmerge_fromfile (char *hashes_filename, char *descriptors_filename, void(*progress_callback)(int, int), void(*event_callback)(int, char*), volatile bool *stop)
{
	FILE *fd_h, *fd_d;
	bool ret = false;
	
	fd_h = fopen (hashes_filename, "r");
	if (fd_h == NULL)
	{
		log_add ("Cannot open %s", hashes_filename);
		goto error;
	}

	fd_d = fopen (descriptors_filename, "r");
	if (fd_d == NULL)
	{
		log_add ("Cannot open %s", descriptors_filename);
		fclose (fd_h);
		goto error;
	}

	//if (event_callback) event_callback(9, hashes_url);	// parsing xepgdb
	if (event_callback) event_callback(0, NULL);	// turn on progress bar
	ret = dbmerge_merge(fd_h, fd_d, progress_callback);
	if (event_callback) event_callback(1, NULL);	// turn off progress bar

	fclose(fd_h);
	fclose(fd_d);

error:
	return ret;
}