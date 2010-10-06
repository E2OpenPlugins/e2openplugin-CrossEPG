%module crossepg
%{

#include "../common.h"
#include "../version.h"
#include "core/log.h"
#include "epgdb/epgdb.h"
#include "epgdb/epgdb_aliases.h"
#include "epgdb/epgdb_channels.h"
#include "epgdb/epgdb_index.h"
#include "epgdb/epgdb_search.h"
#include "epgdb/epgdb_titles.h"
%}

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned int time_t;

%include "core/log.h"
%include "epgdb/epgdb_aliases.h"
%include "epgdb/epgdb_channels.h"
%include "epgdb/epgdb_index.h"
%include "epgdb/epgdb_search.h"
%include "epgdb/epgdb_titles.h"
%include "epgdb/epgdb.h"

%pythoncode %{
import os
import re

def epgdb_get_dbroot():
	if os.path.exists("/usr/crossepg"):
		homedir = "/usr/crossepg"
	elif os.path.exists("/usr/crossepg"):
		homedir = "/var/crossepg"
	else:
		return False
		
	dbroot = "/hdd/crossepg"
	try:
		f = open("%s/crossepg.config" % (homedir), "r")
	except Exception, e:
		return dbroot
		
	entryRe = re.compile(r"db_root=(.*)")
		
	for line in f.readlines(): 
		entry = re.findall(entryRe, line)
		if entry:
			dbroot = entry[0].strip()
	
	f.close()
	return dbroot
	
def IS_UTF8(flags):
	return flags & 0x01
	
def SET_UTF8(flags):
	return flags | 0x01

def UNSET_UTF8(flags):
	return flags & (~0x01)
%}
