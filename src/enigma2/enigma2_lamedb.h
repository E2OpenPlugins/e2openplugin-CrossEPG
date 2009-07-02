#ifndef _ENIGMA2_LAMEDB_H_
#define _ENIGMA2_LAMEDB_H_

bool enigma2_lamedb_read (char *filename);
bool enigma2_lamedb_exist (int nid, int tsid, int sid);
int enigma2_lamedb_get_namespace (int nid, int tsid, int sid);

#endif //_ENIGMA2_LAMEDB_H_
