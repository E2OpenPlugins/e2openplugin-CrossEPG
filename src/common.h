#ifndef _COMMON_H_
#define _COMMON_H_

typedef enum type_bool {false = 0, true} bool;

#include "version.h"

#define MAX_OTV_LOOP_CYCLES			90000
#define MAX_BAT_LOOP_CYCLES			200
#define DEFAULT_LOG_FILE			"/media/crossepg.log"
#define DEFAULT_DWNL_LOG_FILE		"/media/crossepg.downloader.log"
#define DEFAULT_SYNC_LOG_FILE		"/media/crossepg.sync.log"
#define DEFAULT_LAUNCHER_LOG_FILE	"/media/crossepg.launcher.log"
#define DEFAULT_CONFIG_FILE			"crossepg.config"
#define DEFAULT_DICTIONARY_FILE		"crossepg.dictionary"
#define DEFAULT_DWNL_FILE			"crossepg.downloader.plugin"
#define DEFAULT_SYNC_FILE			"crossepg.sync.plugin"
#define DEFAULT_SYNC_HOURS			12
#define DEFAULT_OTV_PROVIDER		"skyit_hotbird_13.0"

#define DEFAULT_DB_ROOT				"/hdd/crossepg"
#define DEFAULT_IMPORT_ROOT			"/hdd/crossepg/import"
#define DEFAULT_EPG_DAT				"/hdd/crossepg/ext.epg.dat"
#define DEFAULT_DEMUXER				"/dev/dvb/adapter0/demux0"
#define DEFAULT_LAMEDB				"/etc/enigma2/lamedb"

#ifdef MEMORYMON
void *_malloc (unsigned int size);
void _free (void *value);
void memory_stats ();
#else
#define _malloc malloc
#define _free free
static inline void memory_stats() {}
#endif
#endif // _COMMON_H_
