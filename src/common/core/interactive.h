#ifndef _INTERACTIVE_H_
#define _INTERACTIVE_H_

#define CMD_SAVE "SAVE"
#define CMD_QUIT "QUIT"
#define CMD_DEMUXER "DEMUXER"
#define CMD_DOWNLOAD "DOWNLOAD"
#define CMD_LAMEDB "LAMEDB"
#define CMD_EPGDAT "EPGDAT"
#define CMD_CONVERT "CONVERT"
#define CMD_TEXT "TEXT"
#define CMD_WAIT "WAIT"
#define CMD_STOP "STOP"

#define ACTION_OK		"OK"
#define ACTION_START	"START"
#define ACTION_ERROR	"ERROR"
#define ACTION_SIZE		"SIZE"
#define ACTION_CHANNELS	"CHANNELS"
#define ACTION_END		"END"
#define ACTION_TYPE		"TYPE"
#define ACTION_PROGRESS	"PROGRESS"
#define ACTION_READY	"READY"

void interactive_enable ();
void interactive_disable ();
void interactive_send (char *action);
void interactive_send_text (char *action, char *value);
void interactive_send_int (char *action, int value);

#endif // _INTERACTIVE_H_
