#ifndef _LOG_H_
#define _LOG_H_

void log_enable ();
void log_disable ();
bool log_open (char *db_root);
void log_banner (char *app_name);
void log_close ();
void log_add (char *message, ...);

#endif // _LOG_H_
