#ifndef _ENIGMA2_HASH_H_
#define _ENIGMA2_HASH_H_

typedef struct enigma2_hash_s
{
	uint32_t hash;
	unsigned char *data;
	unsigned char size;
	int use_count;
	struct enigma2_hash_s 	*prev;
	struct enigma2_hash_s 	*next;
} enigma2_hash_t;

void enigma2_hash_init ();
bool enigma2_hash_add (uint32_t hash, unsigned char *data, unsigned char size);
void enigma2_hash_clean ();
int enigma2_hash_count ();
enigma2_hash_t *enigma2_hash_get_last (unsigned short int prefix);
enigma2_hash_t *enigma2_hash_get_first (unsigned short int prefix);
void enigma2_hash_sort ();

#endif //_ENIGMA2_HASH_H_
