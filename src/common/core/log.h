#ifndef _LOG_H_
#define _LOG_H_

void log_enable ();
void log_disable ();
bool log_open (char *file, char *plugin_name);
void log_close ();
void log_add (char *message, ...);

#endif // _LOG_H_
