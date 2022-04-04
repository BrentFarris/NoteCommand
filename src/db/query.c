#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <db/query.h>

int query_run(sqlite3* db, const char* format, ...) {
	char query[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(query, 1024, format, args);
	va_end(args);
	char* err = NULL;
	int res = sqlite3_exec(db, query, NULL, NULL, &err);
	if (res == SQLITE_ABORT || err != NULL) {
		// TODO:  Log the err
		return QUERY_ERR;
	}
	return QUERY_OK;
}

int query_run_cb(sqlite3* db, int (*callback)(void*, int, char**, char**),
	void* state, const char* format, ...)
{
	char query[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(query, 1024, format, args);
	va_end(args);
	char* err = NULL;
	int res = sqlite3_exec(db, query, callback, state, &err);
	if (res == SQLITE_ABORT || err != NULL) {
		// TODO:  Log the err
		//printf("%s", sqlite3_errstr(res));
		return QUERY_ERR;
	}
	return QUERY_OK;
}
