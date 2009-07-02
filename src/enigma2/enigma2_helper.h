#ifndef _ENIGMA2_HELPER_H_
#define _ENIGMA2_HELPER_H_

bool enigma2_helper_get_channel (int *nid, int *tsid, int *sid);
bool enigma2_helper_change_channel (int nid, int tsid, int sid, bool with_check);

#endif //_ENIGMA2_HELPER_H_
