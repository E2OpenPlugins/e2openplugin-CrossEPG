#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "../../common.h"

#include "epgdb.h"
#include "epgdb_index.h"

static epgdb_index_t *index_first[65536];
static epgdb_index_t *index_last[65536];
static epgdb_index_t *index_empties_first;
static epgdb_index_t *index_empties_last;

int epgdb_index_count ()
{
	int i;
	int count = 0;
	for (i=0; i<65536;i++)
	{
		epgdb_index_t *tmp = index_first[i];
		while (tmp != NULL)
		{
			count++;
			tmp = tmp->next;
		}
	}
	
	return count;
}

int epgdb_index_empties_count ()
{
	int count = 0;
	epgdb_index_t *tmp = index_empties_first;
	while (tmp != NULL)
	{
		count++;
		tmp = tmp->next;
	}
	
	return count;
}

epgdb_index_t *epgdb_index_get_first (unsigned short int key) { return index_first[key]; }
epgdb_index_t *epgdb_index_empties_get_first () { return index_empties_first; }
void epgdb_index_empties_set_first (epgdb_index_t *index) { index_empties_first = index; }
void epgdb_index_empties_set_last (epgdb_index_t *index) { index_empties_last = index; }

void epgdb_index_init ()
{
	int i;
	for (i=0; i<65536; i++)
	{
		index_first[i] = NULL;
		index_last[i] = NULL;
	}
	index_empties_first = NULL;
	index_empties_first = NULL;
}

void epgdb_index_clean ()
{
	int i;
	for (i=0; i<65536;i++)
	{
		epgdb_index_t *index = index_first[i];
		while (index != NULL)
		{
			epgdb_index_t *tmp = index;
			index = index->next;
			_free (tmp);
		}
		index_first[i] = NULL;
		index_last[i] = NULL;
	}
	
	epgdb_index_t *index = index_empties_first;
	while (index != NULL)
	{
		epgdb_index_t *tmp = index;
		index = index->next;
		_free (tmp);
	}
	index_empties_first = NULL;
	index_empties_last = NULL;
}

epgdb_index_t *epgdb_index_get_by_crc_length (uint32_t crc, unsigned short int length)
{
	epgdb_index_t *tmp = index_first[crc & 0xffff];
	
	while (tmp != NULL)
	{
		if ((crc == tmp->crc) && (length == tmp->length)) return tmp;
		tmp = tmp->next;
	}
	
	return NULL;
}

epgdb_index_t *epgdb_index_add (uint32_t crc, unsigned short int length, bool *added)
{
	epgdb_index_t *tmp = index_first[crc & 0xffff];
	bool exist = false;
	*added = false;
	
	while (tmp != NULL)
	{
		if ((crc == tmp->crc) && (length == tmp->length)) return tmp;
		tmp = tmp->next;
	}
	*added = true;
	tmp = index_empties_first;
	while (tmp != NULL)
	{
		if (tmp->length >= length)
		{
			if (tmp->prev != NULL) tmp->prev->next = tmp->next;
			if (tmp->next != NULL) tmp->next->prev = tmp->prev;
			if (index_empties_first == tmp) index_empties_first = tmp->next;
			if (index_empties_last == tmp) index_empties_first = tmp->prev;
			exist = true;
			break;
		}
		tmp = tmp->next;
	}
	
	if (!exist) tmp = _malloc (sizeof (epgdb_index_t));
	tmp->crc = crc;
	tmp->length = length;
	tmp->seek = 0;
	
	if (index_last[crc & 0xffff] == NULL)
	{
		tmp->prev = NULL;
		tmp->next = NULL;
		index_first[crc & 0xffff] = tmp;
		index_last[crc & 0xffff] = tmp;
	}
	else
	{
		index_last[crc & 0xffff]->next = tmp;
		tmp->prev = index_last[crc & 0xffff];
		tmp->next = NULL;
		index_last[crc & 0xffff] = tmp;
	}
	return tmp;
}

void epgdb_index_mark_all_as_unused ()
{
	int i;
	for (i=0; i<65536;i++)
	{
		epgdb_index_t *tmp = index_first[i];
		while (tmp != NULL)
		{
			tmp->used = false;
			tmp = tmp->next;
		}
	}
}

void epgdb_index_mark_as_used (uint32_t crc, unsigned short int length)
{
	epgdb_index_t *tmp = index_first[crc & 0xffff];
	
	while (tmp != NULL)
	{
		if ((crc == tmp->crc) && (length == tmp->length))
		{
			tmp->used = true;
			break;
		}
		tmp = tmp->next;
	}
}

void epgdb_index_empty_unused ()
{
	int i;
	for (i=0; i<65536;i++)
	{
		epgdb_index_t *tmp = index_first[i];
		while (tmp != NULL)
		{
			if (tmp->used == false)
			{
				if (tmp->prev != NULL) tmp->prev->next = tmp->next;
				if (tmp->next != NULL) tmp->next->prev = tmp->prev;
				if (index_empties_first == tmp) index_empties_first = tmp->next;
				if (index_empties_last == tmp) index_empties_first = tmp->prev;
				if (index_empties_last == NULL)
				{
					tmp->next = NULL;
					tmp->prev = NULL;
					index_empties_first = tmp;
					index_empties_last = tmp;
				}
				else
				{
					index_empties_last->next = tmp;
					tmp->prev = index_empties_last;
					tmp->next = NULL;
					index_empties_last = tmp;
				}
			}
			tmp = tmp->next;
		}
	}
}

void epgdb_index_empties_add (epgdb_index_t *index)
{
	if (index_empties_last == NULL)
	{
		index->next = NULL;
		index->prev = NULL;
		index_empties_first = index;
		index_empties_last = index;
	}
	else
	{
		index_empties_last->next = index;
		index->prev = index_empties_last;
		index->next = NULL;
		index_empties_last = index;
	}
}
