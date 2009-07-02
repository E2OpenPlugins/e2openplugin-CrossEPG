#include <stdio.h>
#include <strings.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#ifdef STANDALONE
#include <sqlite3.h>
#else
#include "db/db.h"
#include "directory.h"
#endif

#include "../common.h"
#include "../common/core/log.h"

#include "dgs.h"

static sqlite3 *db = NULL;

bool dgs_opendb ()
{
#ifdef STANDALONE
	int ret;
	ret = sqlite3_open ("/var/db.dat", &db);
	if (ret)
	{
		sqlite3_close (db);
		db = NULL;
	}
#else
	db = db_open (type_db);	
#endif
	if (!db) return false;
	return true;
}

void dgs_closedb ()
{
	if (db != NULL)
	{
#ifdef STANDALONE
		sqlite3_close (db);
#else
		db_close (db);	
#endif
		db = NULL;
	}
}

sqlite3 *dgs_getdb ()
{
	return db;
}
