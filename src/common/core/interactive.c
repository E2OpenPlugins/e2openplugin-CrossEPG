#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "../../common.h"

#include "interactive.h"

static bool enabled = false;

void interactive_enable ()
{
	enabled = true;
}
void interactive_disable ()
{
	enabled = false;
}

void interactive_send (char *action)
{
	if (!enabled) return;
	printf ("%s\n", action);
	fflush (stdout);
}

void interactive_send_text (char *action, char *value)
{
	if (!enabled) return;
	printf ("%s %s\n", action, value);
	fflush (stdout);
}

void interactive_send_int (char *action, int value)
{
	if (!enabled) return;
	printf ("%s %d\n", action, value);
	fflush (stdout);
}
