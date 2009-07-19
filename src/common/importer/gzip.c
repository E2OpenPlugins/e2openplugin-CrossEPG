#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../../common.h"
#include "../core/log.h"
#include "zlib.h"

#define BUFLEN      16384

bool gzip_inf (char *source, FILE *dest)
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

