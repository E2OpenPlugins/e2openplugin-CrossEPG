#include <stdio.h>
#include <libxml/xmlreader.h>
#include <libxml/xmlerror.h>
#include <string.h>
#include <time.h>

#include "../../common.h"

#include "../core/log.h"
#include "../epgdb/epgdb.h"
#include "../epgdb/epgdb_channels.h"
#include "../epgdb/epgdb_titles.h"

#include "xmltv_channels.h"
#include "xmltv_parser.h"

static const char *ISO639_associations[][2] =
	{{"aar", "aa"}, {"abk", "ab"}, {"ave", "ae"}, {"afr", "af"}, {"aka", "ak"}, {"amh", "am"}, {"arg", "an"}, {"ara", "ar"},
	{"asm", "as"}, {"ava", "av"}, {"aym", "ay"}, {"aze", "az"}, {"bak", "ba"}, {"bel", "be"}, {"bul", "bg"}, {"bih", "bh"},
	{"bis", "bi"}, {"bam", "bm"}, {"ben", "bn"}, {"tib", "bo"}, {"tib", "bo"}, {"bre", "br"}, {"bos", "bs"}, {"cat", "ca"},
	{"che", "ce"}, {"cha", "ch"}, {"cos", "co"}, {"cre", "cr"}, {"cze", "cs"}, {"chu", "cu"}, {"chv", "cv"}, {"wel", "cy"},
	{"dan", "da"}, {"ger", "de"}, {"div", "dv"}, {"dzo", "dz"}, {"ewe", "ee"}, {"gre", "el"}, {"eng", "en"}, {"epo", "eo"},
	{"spa", "es"}, {"est", "et"}, {"baq", "eu"}, {"per", "fa"}, {"ful", "ff"}, {"fin", "fi"}, {"fij", "fj"}, {"fao", "fo"},
	{"fre", "fr"}, {"fre", "fr"}, {"fry", "fy"}, {"gle", "ga"}, {"gla", "gd"}, {"glg", "gl"}, {"grn", "gn"}, {"guj", "gu"},
	{"glv", "gv"}, {"hau", "ha"}, {"heb", "he"}, {"hin", "hi"}, {"hmo", "ho"}, {"hrv", "hr"}, {"hat", "ht"}, {"hun", "hu"},
	{"arm", "hy"}, {"her", "hz"}, {"ina", "ia"}, {"ind", "id"}, {"ile", "ie"}, {"ibo", "ig"}, {"iii", "ii"}, {"ipk", "ik"},
	{"ido", "io"}, {"ice", "is"}, {"ita", "it"}, {"iku", "iu"}, {"jpn", "ja"}, {"jav", "jv"}, {"geo", "ka"}, {"kon", "kg"},
	{"kik", "ki"}, {"kua", "kj"}, {"kaz", "kk"}, {"kal", "kl"}, {"khm", "km"}, {"kan", "kn"}, {"kor", "ko"}, {"kau", "kr"},
	{"kas", "ks"}, {"kur", "ku"}, {"kom", "kv"}, {"cor", "kw"}, {"kir", "ky"}, {"lat", "la"}, {"ltz", "lb"}, {"lug", "lg"},
	{"lim", "li"}, {"lin", "ln"}, {"lao", "lo"}, {"lit", "lt"}, {"lub", "lu"}, {"lav", "lv"}, {"mlg", "mg"}, {"mah", "mh"},
	{"mao", "mi"}, {"mac", "mk"}, {"mal", "ml"}, {"mon", "mn"}, {"mar", "mr"}, {"may", "ms"}, {"mlt", "mt"}, {"bur", "my"},
	{"nau", "na"}, {"nob", "nb"}, {"nde", "nd"}, {"nep", "ne"}, {"ndo", "ng"}, {"dut", "nl"}, {"nno", "nn"}, {"nor", "no"},
	{"nbl", "nr"}, {"nav", "nv"}, {"nya", "ny"}, {"oci", "oc"}, {"oji", "oj"}, {"orm", "om"}, {"ori", "or"}, {"oss", "os"},
	{"pan", "pa"}, {"pli", "pi"}, {"pol", "pl"}, {"pus", "ps"}, {"por", "pt"}, {"que", "qu"}, {"roh", "rm"}, {"run", "rn"},
	{"rum", "ro"}, {"rum", "ro"}, {"rus", "ru"}, {"kin", "rw"}, {"san", "sa"}, {"srd", "sc"}, {"snd", "sd"}, {"sme", "se"},
	{"sag", "sg"}, {"sin", "si"}, {"slo", "sk"}, {"slv", "sl"}, {"smo", "sm"}, {"sna", "sn"}, {"som", "so"}, {"alb", "sq"},
	{"alb", "sq"}, {"srp", "sr"}, {"ssw", "ss"}, {"sot", "st"}, {"sun", "su"}, {"swe", "sv"}, {"swa", "sw"}, {"tam", "ta"},
	{"tel", "te"}, {"tgk", "tg"}, {"tha", "th"}, {"tir", "ti"}, {"tuk", "tk"}, {"tgl", "tl"}, {"tsn", "tn"}, {"ton", "to"},
	{"tur", "tr"}, {"tso", "ts"}, {"tat", "tt"}, {"twi", "tw"}, {"tah", "ty"}, {"uig", "ug"}, {"ukr", "uk"}, {"urd", "ur"},
	{"uzb", "uz"}, {"ven", "ve"}, {"vie", "vi"}, {"vol", "vo"}, {"wln", "wa"}, {"wol", "wo"}, {"xho", "xh"}, {"yid", "yi"},
	{"yor", "yo"}, {"zha", "za"}, {"chi", "zh"}, {"zul", "zu"}};

#define ISO630_COUNT 188

static bool is_tv = false;
static bool is_programme = false;
static bool is_title = false;
static bool is_title_selected = false;
static bool is_subtitle = false;
static bool is_subtitle_selected = false;
static bool is_desc = false;
static bool is_desc_selected = false;

static char preferred_iso639[3];
static char* current_title = NULL;
static char current_title_iso639[3];
static char* current_subtitle = NULL;
static char current_subtitle_iso639[3];
static char* current_desc = NULL;
static char current_desc_iso639[3];
static char* current_channel = NULL;
static time_t current_starttime = 0;
static time_t current_stoptime = 0;
static time_t current_time = 0;
static int events_count = 0;
static int events_in_future_count = 0;

static time_t mkgmtime(t)
register struct tm	*t;
{
	register short	month, year;
	register time_t	result;
	static int	m_to_d[12] =
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

	month = t->tm_mon;
	year = t->tm_year + month / 12 + 1900;
	month %= 12;
	if (month < 0)
	{
		year -= 1;
		month += 12;
	}
	result = (year - 1970) * 365 + (year - 1969) / 4 + m_to_d[month];
	result = (year - 1970) * 365 + m_to_d[month];
	if (month <= 1)
		year -= 1;
	result += (year - 1968) / 4;
	result -= (year - 1900) / 100;
	result += (year - 1600) / 400;
	result += t->tm_mday;
	result -= 1;
	result *= 24;
	result += t->tm_hour;
	result *= 60;
	result += t->tm_min;
	result *= 60;
	result += t->tm_sec;
	return(result);
}

static time_t iso8601totime (char *strtime)
{
	int gmt_hours, gmt_minutes, gmt_offset;
	bool positivesign;
	struct tm *timeinfo;
	time_t ret = time (NULL);

	//memset (&timeinfo, 0, sizeof (struct tm));
	timeinfo = gmtime (&ret);

	if (sscanf (strtime, "%4d%2d%2d%2d%2d%2d +%2d%2d", &timeinfo->tm_year, &timeinfo->tm_mon, &timeinfo->tm_mday, &timeinfo->tm_hour, &timeinfo->tm_min, &timeinfo->tm_sec, &gmt_hours, &gmt_minutes) == 8)
	{
		positivesign = true;
	}
	else if (sscanf (strtime, "%4d%2d%2d%2d%2d%2d -%2d%2d", &timeinfo->tm_year, &timeinfo->tm_mon, &timeinfo->tm_mday, &timeinfo->tm_hour, &timeinfo->tm_min, &timeinfo->tm_sec, &gmt_hours, &gmt_minutes) == 8)
	{
		positivesign = false;
	}
	else if (sscanf (strtime, "%4d%2d%2d%2d%2d%2d", &timeinfo->tm_year, &timeinfo->tm_mon, &timeinfo->tm_mday, &timeinfo->tm_hour, &timeinfo->tm_min, &timeinfo->tm_sec) == 6)
	{
		positivesign = true;
		gmt_hours = 0;
		gmt_minutes = 0;
	}
	else
		return 0;

	timeinfo->tm_mon -= 1;
	timeinfo->tm_year -= 1900;
	timeinfo->tm_isdst = 0;
	//timeinfo->tm_gmtoff = gmt_offset;

	ret = mkgmtime (timeinfo);
	gmt_offset = (gmt_hours*60*60) + (gmt_minutes*60);
	if (positivesign)
		ret -= gmt_offset;
	else
		ret += gmt_offset;

	return ret;
}

static void xmltv_parser_add_event ()
{
	xmltv_channel_t* channel = NULL;

	if (current_title_iso639[0] == '\0' && current_title_iso639[1] == '\0' && current_title_iso639[2] == '\0')
	{
		current_title_iso639[0] = 'e';
		current_title_iso639[1] = 'n';
		current_title_iso639[2] = 'g';
	}

	if (current_subtitle)
	{
		if (current_desc)
		{
			char *tmp = _malloc (strlen (current_desc) + strlen (current_subtitle) + 2);
			sprintf (tmp, "%s\n%s", current_subtitle, current_desc);
			_free (current_desc);
			current_desc = tmp;
		}
		else
		{
			current_desc = _malloc (strlen (current_subtitle) + 1);
			strcpy(current_desc, current_subtitle);
		}
	}

	while ((channel = xmltv_channels_get_by_id (current_channel, channel)) != NULL)
	{
		epgdb_channel_t *ch = epgdb_channels_add (channel->nid, channel->tsid, channel->sid);
		epgdb_title_t *title = _malloc (sizeof (epgdb_title_t));
		title->event_id = events_count;
		title->start_time = current_starttime;
		title->mjd = epgdb_calculate_mjd (current_starttime);
		title->length = current_stoptime - current_starttime;
		title->genre_id = 0;
		title->flags = 0;
		SET_UTF8(title->flags);
		title->iso_639_1 = current_title_iso639[0];
		title->iso_639_2 = current_title_iso639[1];
		title->iso_639_3 = current_title_iso639[2];

		title = epgdb_titles_add (ch, title);
		epgdb_titles_set_description (title, current_title);
		if (current_desc)
			epgdb_titles_set_long_description (title, current_desc);

		if (current_starttime>=current_time)
		{
			events_in_future_count++;
			// only log for testdebug:
			//log_add ("current_starttime(%f)<current_time(%f)\n", (double) current_starttime, (double) current_time);
		}
	}

	events_count++;
}

static void processNode (xmlTextReaderPtr reader)
{
	const xmlChar *name;

	name = xmlTextReaderConstName (reader);
	if (name)
	{
		if (!is_tv)
		{
			if (xmlTextReaderNodeType (reader) == 1 && strcmp("tv", (char*)name) == 0)
				is_tv = true;
		}
		else
		{
			if (is_programme)
			{
				if (xmlTextReaderNodeType (reader) == 15 && strcmp("programme", (char*)name) == 0)
				{
					is_programme = false;

					if (current_starttime > 0 && current_stoptime > 0 && current_title)
						xmltv_parser_add_event ();

					if (current_title) _free (current_title);
					if (current_subtitle) _free (current_subtitle);
					if (current_desc) _free (current_desc);
					if (current_channel) _free (current_channel);

					current_title = NULL;
					current_subtitle = NULL;
					current_desc = NULL;
					current_channel = NULL;
				}
				else
				{
					if (is_title)
					{
						if (xmlTextReaderNodeType (reader) == 15 && strcmp("title", (char*)name) == 0)
						{
							is_title_selected = false;
							is_title = false;
						}
						else
						{
							xmlChar *title = xmlTextReaderValue (reader);
							if (title && is_title_selected)
							{
								if (current_title) _free (current_title); // theorically not neccessary.. but in case if
								current_title = _malloc (strlen ((char*)title) + 1);
								//current_title[0] = 0x15; // this mean it's an utf-8 string
								strcpy (current_title, (char*)title);
							}
						}
					}
					else if (is_subtitle)
					{
						if (xmlTextReaderNodeType (reader) == 15 && strcmp("sub-title", (char*)name) == 0)
						{
							is_subtitle_selected = false;
							is_subtitle = false;
						}
						else
						{
							xmlChar *subtitle = xmlTextReaderValue (reader);
							if (subtitle && is_subtitle_selected)
							{
								if (current_subtitle) _free (current_subtitle); // theorically not neccessary.. but in case if
								current_subtitle = _malloc (strlen ((char*)subtitle) + 1);
								strcpy (current_subtitle, (char*)subtitle);
							}
						}
					}
					else if (is_desc)
					{
						if (xmlTextReaderNodeType (reader) == 15 && strcmp("desc", (char*)name) == 0)
						{
							is_desc_selected = false;
							is_desc = false;
						}
						else
						{
							xmlChar *desc = xmlTextReaderValue (reader);
							if (desc && is_desc_selected)
							{
								if (current_desc) _free (current_desc); // theorically not neccessary.. but in case if
								current_desc = _malloc (strlen ((char*)desc) + 1);
								//current_desc[0] = 0x15; // this mean it's an utf-8 string
								strcpy (current_desc, (char*)desc);
							}
						}
					}
					else
					{
						if (xmlTextReaderNodeType (reader) == 1 && strcmp("title", (char*)name) == 0)
						{
							if (memcmp (preferred_iso639, current_title_iso639, 3) != 0)
							{
								int i;
								xmlChar *lang;
								current_title_iso639[0] = '\0';
								current_title_iso639[1] = '\0';
								current_title_iso639[2] = '\0';
								lang = xmlTextReaderGetAttribute (reader, xmlCharStrdup("lang"));
								if (lang)
								{
									if (strlen ((char*)lang) == 2)
									{
										for (i=0; i<ISO630_COUNT; i++)
										{
											if (strcmp (ISO639_associations[i][1], (char*)lang) == 0)
												memcpy (current_title_iso639, ISO639_associations[i][0], 3);
										}
									}
									else if (strlen ((char*)lang) == 3)
										memcpy (current_title_iso639, lang, 3);
								}
								is_title_selected = true;
							}
							is_title = true;
						}
						else if (xmlTextReaderNodeType (reader) == 1 && strcmp("sub-title", (char*)name) == 0)
						{
							if (memcmp (preferred_iso639, current_subtitle_iso639, 3) != 0)
							{
								int i;
								xmlChar *lang;
								current_subtitle_iso639[0] = '\0';
								current_subtitle_iso639[1] = '\0';
								current_subtitle_iso639[2] = '\0';
								lang = xmlTextReaderGetAttribute (reader, xmlCharStrdup("lang"));
								if (lang)
								{
									if (strlen ((char*)lang) == 2)
									{
										for (i=0; i<ISO630_COUNT; i++)
										{
											if (strcmp (ISO639_associations[i][1], (char*)lang) == 0)
												memcpy (current_subtitle_iso639, ISO639_associations[i][0], 3);
										}
									}
									else if (strlen ((char*)lang) == 3)
										memcpy (current_subtitle_iso639, lang, 3);
								}
								is_subtitle_selected = true;
							}
							is_subtitle = true;
						}
						else if (xmlTextReaderNodeType (reader) == 1 && strcmp("desc", (char*)name) == 0)
						{
							if (memcmp (preferred_iso639, current_desc_iso639, 3) != 0)
							{
								int i;
								xmlChar *lang;
								current_desc_iso639[0] = '\0';
								current_desc_iso639[1] = '\0';
								current_desc_iso639[2] = '\0';
								lang = xmlTextReaderGetAttribute (reader, xmlCharStrdup("lang"));
								if (lang)
								{
									if (strlen ((char*)lang) == 2)
									{
										for (i=0; i<ISO630_COUNT; i++)
										{
											if (strcmp (ISO639_associations[i][1], (char*)lang) == 0)
												memcpy (current_desc_iso639, ISO639_associations[i][0], 3);
										}
									}
									else if (strlen ((char*)lang) == 3)
										memcpy (current_desc_iso639, lang, 3);
								}
								is_desc_selected = true;
							}
							is_desc = true;
						}
					}
				}
			}
			else
			{
				if (xmlTextReaderNodeType (reader) == 1 && strcmp("programme", (char*)name) == 0)
				{
					xmlChar *starttime, *stoptime, *channel;
					is_programme = true;
					if (current_title) _free (current_title);
					if (current_subtitle) _free (current_subtitle);
					if (current_desc) _free (current_desc);
					if (current_channel) _free (current_channel);
					current_title = NULL;
					current_subtitle = NULL;
					current_desc = NULL;
					current_channel = NULL;
					current_starttime = 0;
					current_stoptime = 0;
					current_title_iso639[0] = '\0';
					current_title_iso639[1] = '\0';
					current_title_iso639[2] = '\0';
					current_subtitle_iso639[0] = '\0';
					current_subtitle_iso639[1] = '\0';
					current_subtitle_iso639[2] = '\0';
					current_desc_iso639[0] = '\0';
					current_desc_iso639[1] = '\0';
					current_desc_iso639[2] = '\0';

					starttime = xmlTextReaderGetAttribute (reader, xmlCharStrdup("start"));
					if (starttime)
						current_starttime = iso8601totime ((char*)starttime);

					stoptime = xmlTextReaderGetAttribute (reader, xmlCharStrdup("stop"));
					if (stoptime)
						current_stoptime = iso8601totime ((char*)stoptime);

					channel = xmlTextReaderGetAttribute (reader, xmlCharStrdup("channel"));
					if (channel)
					{
						current_channel = _malloc (strlen ((char*)channel) + 1);
						strcpy (current_channel, (char*)channel);
					}
				}
				else if (xmlTextReaderNodeType (reader) == 15 && strcmp("tv", (char*)name) == 0)
					is_tv = false;
			}
		}
	}
}

void xmltv_parser_set_iso639 (char *iso639)
{
	memcpy (preferred_iso639, iso639, 3);
}

bool xmltv_parser_import (char *filename, void(*progress_callback)(int, int), volatile bool *stop)
{
	xmlTextReaderPtr reader;
	int ret;
	int filesize = 0;
	FILE *fd;

	log_add ("Parsing %s", filename);

	fd = fopen (filename, "r");
	if (fd)
	{
		fseek (fd, 0, SEEK_END);
		filesize = ftell (fd);
		fclose (fd);
	}

	reader = xmlReaderForFile (filename, NULL, 0);
	if (!reader)
	{
 		log_add ("Unable to open %s\n", filename);
		return false;
	}

	is_tv = false;
	is_programme = false;
	is_title = false;
	is_title_selected = false;
	is_subtitle = false;
	is_subtitle_selected = false;
	is_desc = false;
	is_desc_selected = false;
	current_title = NULL;
	current_channel = NULL;
	current_starttime = 0;
	current_stoptime = 0;
	current_time = time(NULL);
	events_count = 0;
	events_in_future_count = 0;

	ret = xmlTextReaderRead (reader);
	while (ret == 1)
	{
		if (*stop)
		{
			ret = -1;
			continue;
		}
		processNode (reader);
		ret = xmlTextReaderRead (reader);
		if (progress_callback)
			progress_callback (xmlTextReaderByteConsumed (reader), filesize);
	}

	log_add ("Read %d events", events_count);

	/* clean up */
	if (current_title) _free (current_title);
	if (current_subtitle) _free (current_subtitle);
	if (current_desc) _free (current_desc);
	if (current_channel) _free (current_channel);

	current_title = NULL;
	current_subtitle = NULL;
	current_desc = NULL;
	current_channel = NULL;

	xmlFreeTextReader (reader);

	if (ret != 0)
	{
		xmlErrorPtr error = xmlGetLastError();
		if (error != NULL)
			log_add ("Failed to parse %s (on line %d: %s)\n", filename, error->line, error->message);
		else
			log_add ("Failed to parse %s\n", filename);
		return false;
	}

	if (events_in_future_count == 0)
	{
		log_add ("Failed to parse %s for new events\n", filename);
		return false;
	}

	return true;
}
