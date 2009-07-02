#ifndef _HTTP_H_
#define _HTTP_H_

bool http_get (char *host, char *page, int port, int tempfile, void(*progress_callback)(int, int));

#endif // _HTTP_H_
