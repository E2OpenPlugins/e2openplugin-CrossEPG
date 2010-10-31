#ifndef _DBMERGE_H_
#define _DBMERGE_H_

bool dbmerge_downloader (char *hashes_url, char *descriptors_url, char *dbroot, void(*progress_callback)(int, int), void(*event_callback)(int, char*), volatile bool *stop);

#endif // _DBMERGE_H_
