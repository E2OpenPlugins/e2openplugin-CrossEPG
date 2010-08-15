#ifndef _SCHEDA_H_
#define _SCHEDA_H_

void append_scheda(char *buf, xmlNodePtr node, int bufSize);
char *read_scheda(char *baseUrl, xmlChar *url, char *epgdb_root);

#endif // _SCHEDA_H_
