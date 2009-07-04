#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "db/db.h"
#include "directory.h"

#include "../../common.h"
#include "log.h"

#include "intl.h"

#define MAX_STRINGS 80

static char strings[MAX_STRINGS][256];

void intl_init ()
{
	strcpy (strings[LANGUAGE],				"English");
	strcpy (strings[TIME_HM],				"%H:%M");
	strcpy (strings[DATE_DM],				"%d/%m");
	strcpy (strings[ELAPSED],				"Elapsed");
	strcpy (strings[MISSING1],				"Missing: %d days %d hours %d mintues");
	strcpy (strings[MISSING2],				"Missing: %d hours %d minutes");
	strcpy (strings[MISSING3],				"Missing: %d minutes");
	strcpy (strings[STARTED],				"Started: %d minutes ago");
	strcpy (strings[START],					"%s %d %s at %s (%d minutes)");
	strcpy (strings[START_TODAY],			"Today at %s (%d minutes)");
	strcpy (strings[MENU],					"Menu");
	strcpy (strings[ZAP],					"Zap");
	strcpy (strings[RECORD],				"Record");
	strcpy (strings[CHANGE_GROUP],			"Ch. group");
	strcpy (strings[GO_TO_NOW],				"Got to now");
	strcpy (strings[LG_24],					"+/- 24h");
	strcpy (strings[PAGE_UD],				"Page Up/Down");
	strcpy (strings[MORE_INFO],				"More information");
	strcpy (strings[CONFIGURATION], 		"Configuration");
	strcpy (strings[DATABASE_PATH], 		"Database path");
	strcpy (strings[LOGS_PATH],				"Logs path");
	strcpy (strings[SYNC_HOURS],			"Sync hours");
	strcpy (strings[SYNC_GROUPS],			"Sync groups");
	strcpy (strings[USB_PEN],				"Usb pen (/mnt/usb)");
	strcpy (strings[TEMP_FOLDER],			"Temp folder (/tmp)");
	strcpy (strings[YES],					"Yes");
	strcpy (strings[NO],					"No");
	strcpy (strings[CUSTOM],				"Custom");
	strcpy (strings[HARD_DISK],				"Hard disk (/media)");
	strcpy (strings[START_SYNC],			"Start sync");	
	strcpy (strings[START_DOWNLOADER],		"Start downloader");
	strcpy (strings[SUNDAY],				"Sunday");
	strcpy (strings[MONDAY],				"Monday");
	strcpy (strings[TUESDAY],				"Tuesday");
	strcpy (strings[WEDNESDAY],				"Wednesday");
	strcpy (strings[THURSDAY],				"Thursday");
	strcpy (strings[FRIDAY],				"Friday");
	strcpy (strings[SATURADY],				"Saturday");
	strcpy (strings[JANUARY],				"January");
	strcpy (strings[FEBRUARY],				"February");
	strcpy (strings[MARCH],					"March");
	strcpy (strings[APRIL],					"April");
	strcpy (strings[MAY],					"May");
	strcpy (strings[JUNE],					"June");
	strcpy (strings[JULY],					"July");
	strcpy (strings[AUGUST],				"August");
	strcpy (strings[SEPTEMBER],				"September");
	strcpy (strings[OCTOBER],				"October");
	strcpy (strings[NOVEMBER],				"November");
	strcpy (strings[DECEMBER],				"December");
	strcpy (strings[PROVIDERS],				"Providers");
	strcpy (strings[ERROR],					"Error");
	strcpy (strings[SERIOUS_ERROR],			"Seriours error");
	strcpy (strings[CANNOT_OPEN_DGSDB],		"Cannot open DGS database");
	strcpy (strings[CANNOT_OPEN_LOG_FILE],	"Cannot open log file. Please check your configuration");
	strcpy (strings[NO_FAV_GROUPS],			"No fav groups founded. The application cannot work without fav groups");
	strcpy (strings[ERROR_OPEN_EPGDB],		"Error opening EPGDB.\nPlease check your configuration");
	strcpy (strings[ERROR_READ_EPGDB],		"Error reading EPGDB data.\nPlease download data again");
	strcpy (strings[CANNOT_LOAD_PROVIDER],	"Cannot load provider configuration");
	strcpy (strings[CANNOT_LOAD_DICTIONARY],"Cannot load dictionary");
	strcpy (strings[SYNC_EPG],				"Synching internal EPG");
	strcpy (strings[COMPLETED],				"Completed");
	strcpy (strings[READING_CHANNELS],		"Reading channels... (%d)");
	strcpy (strings[READING_TITLES],		"Reading titles... (%s)");
	strcpy (strings[PARSING_TITLES],		"Parsing titles...");
	strcpy (strings[READING_SUMMARIES],		"Reading summaries... (%s)");
	strcpy (strings[PARSING_SUMMARIES],		"Parsing summaries...");
	strcpy (strings[SAVING_DATA],			"Saving data...");
	strcpy (strings[SKIN],					"Skin");
	strcpy (strings[STEP],					"Step");
	strcpy (strings[TODAY],					"Today");
	strcpy (strings[GRID],					"Grid");
	strcpy (strings[LIST],					"List");
	strcpy (strings[LIGHT],					"Light");
	strcpy (strings[SCHEDULER],				"Scheduler");
	strcpy (strings[NO_TITLE],				"No title");
	strcpy (strings[LINKED],				"Linked");
	strcpy (strings[SCHEDULER_ADD],			"Adding event into scheduler");
	strcpy (strings[SCHEDULER_DEL],			"Removing event from scheduler");
	strcpy (strings[LINKING_EVENTS],		"Linking events");
}

static char *intl_trim_spaces (char *text)
{
	char *tmp = text;
	while (tmp[0] == ' ') tmp++;
	while (strlen (tmp) > 1)
		if (tmp[strlen (tmp) - 1] == ' ') tmp[strlen (tmp) - 1] = '\0';
		else break;
	
	if (tmp[0] == ' ') tmp[0] = '\0';
	return tmp;
}

bool intl_read (char *file)
{
	FILE *fd = NULL;
	char line[512];
	char *key;
	char *tmp_value;
	char value[256];
	
	fd = fopen (file, "r");
	if (!fd) 
		return false;
	
	while (fgets (line, sizeof (line), fd)) 
	{
		//if (sscanf (line, "%[^#=]=%s\n", key, value) != 2)
		//	continue;

		char *sep = strchr (line, '=');
		if (sep != NULL)
		{
			int i, j = 0;
			int offset = sep - line;
			line[offset] = '\0';
			key = line;
			tmp_value = line + offset + 1;
			for (i=0; i<strlen (tmp_value); i++)
			{
				if (tmp_value[i] == '\\')
				{
					if (tmp_value[i+1] == 'n')
					{
						value[j] = '\n';
						i++;
					}
					if (tmp_value[i+1] == '\\')
					{
						value[j] = '\\';
						i++;
					}
				}
				else
					value[j] = tmp_value[i];
					
				j++;
			}
			value[j] = '\0';

			key = intl_trim_spaces (key);

			if (strcmp ("LANGUAGE", key) == 0)
				strcpy (strings[LANGUAGE], value);
			else if (strcmp ("TIME_HM", key) == 0)
				strcpy (strings[TIME_HM], value);
			else if (strcmp ("DATE_DM", key) == 0)
				strcpy (strings[DATE_DM], value);
			else if (strcmp ("ELAPSED", key) == 0)
				strcpy (strings[ELAPSED], value);
			else if (strcmp ("MISSING1", key) == 0)
				strcpy (strings[MISSING1], value);
			else if (strcmp ("MISSING2", key) == 0)
				strcpy (strings[MISSING2], value);
			else if (strcmp ("MISSING3", key) == 0)
				strcpy (strings[MISSING3], value);
			else if (strcmp ("STARTED", key) == 0)
				strcpy (strings[STARTED], value);
			else if (strcmp ("START", key) == 0)
				strcpy (strings[START], value);
			else if (strcmp ("START_TODAY", key) == 0)
				strcpy (strings[START_TODAY], value);
			else if (strcmp ("MENU", key) == 0)
				strcpy (strings[MENU], value);
			else if (strcmp ("ZAP", key) == 0)
				strcpy (strings[ZAP], value);
			else if (strcmp ("RECORD", key) == 0)
				strcpy (strings[RECORD], value);
			else if (strcmp ("CHANGE_GROUP", key) == 0)
				strcpy (strings[CHANGE_GROUP], value);
			else if (strcmp ("GO_TO_NOW", key) == 0)
				strcpy (strings[GO_TO_NOW], value);
			else if (strcmp ("LG_24", key) == 0)
				strcpy (strings[LG_24], value);
			else if (strcmp ("PAGE_UD", key) == 0)
				strcpy (strings[PAGE_UD], value);
			else if (strcmp ("MORE_INFO", key) == 0)
				strcpy (strings[MORE_INFO], value);
			else if (strcmp ("CONFIGURATION", key) == 0)
				strcpy (strings[CONFIGURATION], value);
			else if (strcmp ("DATABASE_PATH", key) == 0)
				strcpy (strings[DATABASE_PATH], value);
			else if (strcmp ("LOGS_PATH", key) == 0)
				strcpy (strings[LOGS_PATH], value);
			else if (strcmp ("SYNC_HOURS", key) == 0)
				strcpy (strings[SYNC_HOURS], value);
			else if (strcmp ("SYNC_GROUPS", key) == 0)
				strcpy (strings[SYNC_GROUPS], value);
			else if (strcmp ("USB_PEN", key) == 0)
				strcpy (strings[USB_PEN], value);
			else if (strcmp ("TEMP_FOLDER", key) == 0)
				strcpy (strings[TEMP_FOLDER], value);
			else if (strcmp ("YES", key) == 0)
				strcpy (strings[YES], value);
			else if (strcmp ("NO", key) == 0)
				strcpy (strings[NO], value);
			else if (strcmp ("CUSTOM", key) == 0)
				strcpy (strings[CUSTOM], value);
			else if (strcmp ("HARD_DISK", key) == 0)
				strcpy (strings[HARD_DISK], value);
			else if (strcmp ("START_SYNC", key) == 0)
				strcpy (strings[START_SYNC], value);
			else if (strcmp ("START_DOWNLOADER", key) == 0)
				strcpy (strings[START_DOWNLOADER], value);
			else if (strcmp ("SUNDAY", key) == 0)
				strcpy (strings[SUNDAY], value);
			else if (strcmp ("MONDAY", key) == 0)
				strcpy (strings[MONDAY], value);
			else if (strcmp ("TUESDAY", key) == 0)
				strcpy (strings[TUESDAY], value);
			else if (strcmp ("WEDNESDAY", key) == 0)
				strcpy (strings[WEDNESDAY], value);
			else if (strcmp ("THURSDAY", key) == 0)
				strcpy (strings[THURSDAY], value);
			else if (strcmp ("FRIDAY", key) == 0)
				strcpy (strings[FRIDAY], value);
			else if (strcmp ("SATURADY", key) == 0)
				strcpy (strings[SATURADY], value);
			else if (strcmp ("JANUARY", key) == 0)
				strcpy (strings[JANUARY], value);
			else if (strcmp ("FEBRUARY", key) == 0)
				strcpy (strings[FEBRUARY], value);
			else if (strcmp ("MARCH", key) == 0)
				strcpy (strings[MARCH], value);
			else if (strcmp ("APRIL", key) == 0)
				strcpy (strings[APRIL], value);
			else if (strcmp ("MAY", key) == 0)
				strcpy (strings[MAY], value);
			else if (strcmp ("JUNE", key) == 0)
				strcpy (strings[JUNE], value);
			else if (strcmp ("JULY", key) == 0)
				strcpy (strings[JULY], value);
			else if (strcmp ("AUGUST", key) == 0)
				strcpy (strings[AUGUST], value);
			else if (strcmp ("SEPTEMBER", key) == 0)
				strcpy (strings[SEPTEMBER], value);
			else if (strcmp ("OCTOBER", key) == 0)
				strcpy (strings[OCTOBER], value);
			else if (strcmp ("NOVEMBER", key) == 0)
				strcpy (strings[NOVEMBER], value);
			else if (strcmp ("DECEMBER", key) == 0)
				strcpy (strings[DECEMBER], value);
			else if (strcmp ("PROVIDERS", key) == 0)
				strcpy (strings[PROVIDERS], value);
			else if (strcmp ("ERROR", key) == 0)
				strcpy (strings[ERROR], value);
			else if (strcmp ("SERIOUS_ERROR", key) == 0)
				strcpy (strings[SERIOUS_ERROR], value);
			else if (strcmp ("CANNOT_OPEN_DGSDB", key) == 0)
				strcpy (strings[CANNOT_OPEN_DGSDB], value);
			else if (strcmp ("CANNOT_OPEN_LOG_FILE", key) == 0)
				strcpy (strings[CANNOT_OPEN_LOG_FILE], value);
			else if (strcmp ("NO_FAV_GROUPS", key) == 0)
				strcpy (strings[NO_FAV_GROUPS], value);
			else if (strcmp ("ERROR_OPEN_EPGDB", key) == 0)
				strcpy (strings[ERROR_OPEN_EPGDB], value);
			else if (strcmp ("ERROR_READ_EPGDB", key) == 0)
				strcpy (strings[ERROR_READ_EPGDB], value);
			else if (strcmp ("CANNOT_LOAD_PROVIDER", key) == 0)
				strcpy (strings[CANNOT_LOAD_PROVIDER], value);
			else if (strcmp ("CANNOT_LOAD_DICTIONARY", key) == 0)
				strcpy (strings[CANNOT_LOAD_DICTIONARY], value);
			else if (strcmp ("SYNC_EPG", key) == 0)
				strcpy (strings[SYNC_EPG], value);
			else if (strcmp ("READING_CHANNELS", key) == 0)
				strcpy (strings[READING_CHANNELS], value);
			else if (strcmp ("READING_TITLES", key) == 0)
				strcpy (strings[READING_TITLES], value);
			else if (strcmp ("PARSING_TITLES", key) == 0)
				strcpy (strings[PARSING_TITLES], value);
			else if (strcmp ("READING_SUMMARIES", key) == 0)
				strcpy (strings[READING_SUMMARIES], value);
			else if (strcmp ("PARSING_SUMMARIES", key) == 0)
				strcpy (strings[PARSING_SUMMARIES], value);
			else if (strcmp ("SAVING_DATA", key) == 0)
				strcpy (strings[SAVING_DATA], value);
			else if (strcmp ("COMPLETED", key) == 0)
				strcpy (strings[COMPLETED], value);
			else if (strcmp ("SKIN", key) == 0)
				strcpy (strings[SKIN], value);
			else if (strcmp ("STEP", key) == 0)
				strcpy (strings[STEP], value);
			else if (strcmp ("TODAY", key) == 0)
				strcpy (strings[TODAY], value);
			else if (strcmp ("GRID", key) == 0)
				strcpy (strings[GRID], value);
			else if (strcmp ("LIST", key) == 0)
				strcpy (strings[LIST], value);
			else if (strcmp ("LIGHT", key) == 0)
				strcpy (strings[LIGHT], value);
			else if (strcmp ("SCHEDULER", key) == 0)
				strcpy (strings[SCHEDULER], value);
			else if (strcmp ("NO_TITLE", key) == 0)
				strcpy (strings[NO_TITLE], value);
			else if (strcmp ("LINKED", key) == 0)
				strcpy (strings[LINKED], value);
			else if (strcmp ("SCHEDULER_ADD", key) == 0)
				strcpy (strings[SCHEDULER_ADD], value);
			else if (strcmp ("SCHEDULER_DEL", key) == 0)
				strcpy (strings[SCHEDULER_DEL], value);
			else if (strcmp ("LINKING_EVENTS", key) == 0)
				strcpy (strings[LINKING_EVENTS], value);
		}
	}
	
	fclose (fd);
	
	return true;
}

char *intl (intl_strings_t sid)
{
	return strings[sid];
}
