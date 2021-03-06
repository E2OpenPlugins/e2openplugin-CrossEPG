#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdbool.h>

#include "log.h"

static FILE *fd = NULL;
static int enabled = true;

void log_enable ()
{
	enabled = true;
}
void log_disable ()
{
	enabled = false;
}

bool log_open (char *file, char *info)
{	
	if (!enabled)
	return true;
	
	if (file != NULL)
		fd = fopen (file, "w");
	
	log_add (info);
	
	return (fd != NULL);
}

void log_close ()
{
	if (fd != NULL)
	fclose (fd);
}

void log_add (char *message, ...)
{
	if (!enabled)
	return;
	
	va_list args;
	char msg[16*1024];
	time_t now_time;
	struct tm *loctime;

	now_time = time (NULL);
	loctime = localtime (&now_time);
	strftime (msg, 255, "%d/%m/%Y %H:%M:%S ", loctime);
	
	//fwrite (msg, strlen (msg), 1, stdout);
	if (fd != NULL)
	fwrite (msg, strlen (msg), 1, fd);

	va_start (args, message);
	vsnprintf (msg, 16*1024, message, args);
	va_end (args);
	msg[(16*1024)-1] = '\0';
	
	//fwrite (msg, strlen (msg), 1, stdout);
	//fwrite ("\n", 1, 1, stdout);
	//fflush (stdout);
	if (fd != NULL)
	{
		fwrite (msg, strlen (msg), 1, fd);
		fwrite ("\n", 1, 1, fd);
		fflush (fd);
	}
}
