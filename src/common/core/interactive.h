#ifndef _INTERACTIVE_H_
#define _INTERACTIVE_H_

#define CMD_SAVE "SAVE"
#define CMD_QUIT "QUIT"
#define CMD_DEMUXER "DEMUXER"
#define CMD_FRONTEND "FRONTEND"
#define CMD_DOWNLOAD "DOWNLOAD"
#define CMD_LAMEDB "LAMEDB"
#define CMD_EPGDAT "EPGDAT"
#define CMD_CONVERT "CONVERT"
#define CMD_IMPORT "IMPORT"
#define CMD_DEFRAGMENT "DEFRAGMENT"
#define CMD_TEXT "TEXT"
#define CMD_WAIT "WAIT"
#define CMD_STOP "STOP"
#define CMD_OPEN "OPEN"
#define CMD_CLOSE "CLOSE"

#define ACTION_OK		"OK"
#define ACTION_START	"START"
#define ACTION_ERROR	"ERROR"
#define ACTION_SIZE		"SIZE"
#define ACTION_CHANNELS	"CHANNELS"
#define ACTION_END		"END"
#define ACTION_TYPE		"TYPE"
#define ACTION_PROGRESS	"PROGRESS"
#define ACTION_FILE		"FILE"
#define ACTION_URL		"URL"
#define ACTION_READY	"READY"

#define INFO_HEADERSDB_SIZE					"HEADERSDB_SIZE"
#define INFO_DESCRIPTORSDB_SIZE				"DESCRIPTORSDB_SIZE"
#define INFO_INDEXESDB_SIZE					"INDEXESDB_SIZE"
#define INFO_ALIASESDB_SIZE					"ALIASESDB_SIZE"
#define INFO_TOTAL_SIZE						"TOTAL_SIZE"
#define INFO_CHANNELS_COUNT					"CHANNELS_COUNT"
#define INFO_EVENTS_COUNT					"EVENTS_COUNT"
#define INFO_HASHES_COUNT					"HASHES_COUNT"
#define INFO_DESCRIPTORS_SIZE				"DESCRIPTORS_SIZE"
#define INFO_DESCRIPTORSDB_UNUSED_SPACE		"DESCRIPTORSDB_UNUSED_SPACE"
#define INFO_CREATION_TIME					"CREATION_TIME"
#define INFO_UPDATE_TIME					"UPDATE_TIME"
#define INFO_VERSION						"VERSION"

void interactive_enable ();
void interactive_disable ();
void interactive_send (char *action);
void interactive_send_text (char *action, char *value);
void interactive_send_int (char *action, int value);

#endif // _INTERACTIVE_H_
