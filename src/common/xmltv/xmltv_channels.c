#include <stdio.h>
#include <libxml/xmlreader.h>
#include <string.h>
#include <time.h>

#include "../../common.h"

#include "../core/log.h"

#include "xmltv_channels.h"

static bool isChannels = false;
static bool isChannel = false;
static int channelsCount = 0;

static xmlChar *current_id;

static xmltv_channel_t *xml_channels_first = NULL;
static xmltv_channel_t *xml_channels_last = NULL;

static void processNode (xmlTextReaderPtr reader)
{
	const xmlChar *name;

	name = xmlTextReaderConstName (reader);
	if (name)
	{
		if (!isChannels)
		{
			if (xmlTextReaderNodeType (reader) == 1 && strcmp("channels", (char*)name) == 0)
				isChannels = true;
		}
		else
		{
			if (!isChannel)
			{
				if (xmlTextReaderNodeType (reader) == 1 && strcmp("channel", (char*)name) == 0)
				{
					current_id = xmlTextReaderGetAttribute (reader, xmlCharStrdup("id"));

					isChannel = true;
				}
				else if (xmlTextReaderNodeType (reader) == 15 && strcmp("channels", (char*)name) == 0)
					isChannels = false;
			}
			else
			{
				if (xmlTextReaderNodeType (reader) == 15 && strcmp("channel", (char*)name) == 0)
					isChannel = false;
				else
				{
					xmlChar *value = xmlTextReaderValue (reader);
					if (current_id && value)
					{
						int nid, tsid, sid, trash1, trash2, trash3, trash4, trash5, trash6, trash7;

						if (sscanf ((char*)value, "%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n", &trash1, &trash2, &trash3, &sid, &tsid, &nid, &trash4, &trash5, &trash6, &trash7) == 10)
						{
							xmltv_channel_t *tmp = _malloc (sizeof (xmltv_channel_t));
							tmp->nid = nid;
							tmp->tsid = tsid;
							tmp->sid = sid;
							tmp->id = _malloc (strlen ((char*)current_id) + 1);
							tmp->prev = NULL;
							tmp->next = NULL;
							strcpy (tmp->id, (char*)current_id);

							if (xml_channels_last == NULL)
							{
								xml_channels_first = tmp;
								xml_channels_last = tmp;
							}
							else
							{
								tmp->prev = xml_channels_last;
								xml_channels_last->next = tmp;
								xml_channels_last = tmp;
							}
							channelsCount++;
						}
					}
				}
			}
		}
	}
}

void xmltv_channels_init ()
{
	xml_channels_first = NULL;
	xml_channels_last = NULL;
}

void xmltv_channels_cleanup ()
{
	if (xml_channels_first != NULL)
	{
		xmltv_channel_t *tmp = xml_channels_first;
		while (tmp != NULL)
		{
			xmltv_channel_t *next = tmp->next;

			if (tmp->id != NULL)
				_free (tmp->id);

			_free (tmp);

			tmp = next;
		}
	}
	xml_channels_first = NULL;
	xml_channels_last = NULL;
}

xmltv_channel_t* xmltv_channels_get_by_id (char *id, xmltv_channel_t* last)
{
	if (xml_channels_first != NULL)
	{
		xmltv_channel_t *tmp = xml_channels_first;
		if (last) tmp = last->next;
		while (tmp != NULL)
		{
			if (strcasecmp (tmp->id, id) == 0)
				return tmp;

			tmp = tmp->next;
		}
	}
	return NULL;
}

bool xmltv_channels_load (char *filename)
{
	xmlTextReaderPtr reader;
	int ret;

	log_add ("Reading channels from %s", filename);

	reader = xmlReaderForFile (filename, NULL, 0);
	if (!reader)
	{
 		log_add ("Unable to open %s", filename);
		return false;
	}
	isChannels = false;
	isChannel = false;
	channelsCount = 0;

	ret = xmlTextReaderRead (reader);
	while (ret == 1)
	{
		processNode (reader);
		ret = xmlTextReaderRead (reader);
	}

	xmlFreeTextReader (reader);

	log_add ("Read %d channels", channelsCount);

	if (ret != 0)
	{
		log_add ("Failed to parse %s", filename);
		return false;
	}

	return true;
}
