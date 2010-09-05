#ifndef _XMLTV_DOWNLOADER_H_
#define _XMLTV_DOWNLOADER_H_

bool xmltv_downloader_channels (char *url, char *dbroot, void(*progress_callback)(int, int), void(*event_callback)(int, char*), volatile bool *stop);
bool xmltv_downloader_events (char *url, char *dbroot, void(*progress_callback)(int, int), void(*event_callback)(int, char*), volatile bool *stop);

#endif // _XMLTV_DOWNLOADER_H_
