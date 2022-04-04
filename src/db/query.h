#ifndef DB_QUERY_H
#define DB_QUERY_H

#include <ext/sqlite3.h>

#define QUERY_OK	0
#define QUERY_ERR	1

#define querycbtype(stateType, db, cb, state, format, ...)	\
	((int(*)(sqlite3*, int (*)(stateType, int, char**, char**), stateType, const char*, ...))query_run_cb) \
	(db, cb, state, format, __VA_ARGS__)

int query_run(sqlite3* db, const char* format, ...);
int query_run_cb(sqlite3* db, int (*callback)(void*, int, char**, char**),
	void* state, const char* format, ...);

#endif