#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

#include "../common.h"

#include "enigma2_hash.h"

static enigma2_hash_t *hashes[65536];

void enigma2_hash_init ()
{
	int i;
	for (i=0; i<65536; i++)
		hashes[i] = NULL;
}

bool enigma2_hash_add (uint32_t hash, unsigned char *data, unsigned char size)
{
	enigma2_hash_t *new = _malloc (sizeof (enigma2_hash_t));
	new->hash = hash;
	new->data = data;
	new->size = size;
	new->use_count = 1;
	new->prev = NULL;
	new->next = NULL;
	
	if (hashes[hash & 0xffff] == NULL) hashes[hash & 0xffff] = new;
	else
	{
		enigma2_hash_t *tmp = hashes[hash & 0xffff];
		while (true)
		{
			if (tmp->hash == hash)
			{
				tmp->use_count++;
				_free (new);
				return false;
			}
			if (tmp->next == NULL) break;
			tmp = tmp->next;
		}
		
		new->prev = tmp;
		tmp->next = new;
	}
	
	return true;
}

void enigma2_hash_clean ()
{
	int i;
	for (i=0; i<65536;i++)
	{
		enigma2_hash_t *hash = hashes[i];
		while (hash != NULL)
		{
			enigma2_hash_t *tmp = hash;
			hash = hash->next;
			_free (tmp->data);
			_free (tmp);
		}
		hashes[i] = NULL;
	}
}

int enigma2_hash_count ()
{
	int i;
	int count = 0;
	for (i=0; i<65536;i++)
	{
		enigma2_hash_t *hash = hashes[i];
		while (hash != NULL)
		{
			count++;
			hash = hash->next;
		}
	}
	
	return count;
}

enigma2_hash_t *enigma2_hash_get_last (unsigned short int prefix)
{
	enigma2_hash_t *hash = hashes[prefix];
	if (hash == NULL) return NULL;
	while (true)
	{
		if (hash->next == NULL) break;
		hash = hash->next;
	}
	
	return hash;
}

enigma2_hash_t *enigma2_hash_get_first (unsigned short int prefix)
{
	return hashes[prefix];
}

void enigma2_hash_sort ()
{
	int i;
	for (i=0; i<65536;i++)
	{
		if (hashes[i] == NULL) continue;
		enigma2_hash_t *hash = hashes[i];

		while (hash != NULL)
		{
			enigma2_hash_t *last = enigma2_hash_get_last (i);
			if (hash == last) break;

			do
			{
				if (last->hash < last->prev->hash)
				{
					int use_count = last->use_count;
					unsigned char *tmp_data = last->data;
					unsigned char tmp_size = last->size;
					uint32_t tmp_hash = last->hash;
					last->use_count = last->prev->use_count;
					last->data = last->prev->data;
					last->size = last->prev->size;
					last->hash = last->prev->hash;
					last->prev->use_count = use_count;
					last->prev->data = tmp_data;
					last->prev->size = tmp_size;
					last->prev->hash = tmp_hash;
				}

				last = last->prev;
			}
			while (last != hash);

			hash = hash->next;
		}
	}
}
