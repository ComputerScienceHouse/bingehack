#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#include <mysql.h>

#include "hack.h"
#include "mysql_library.h"

static bool mysql_library_disabled = false;

static const char *nh_dlerror() {
	const char *err = dlerror();
	return err == NULL ? "dylib routine failed with no error message" : err;
}

static void *mysql_function( const char *name ) {
	assert(mysql.handle != NULL);
	void *ret;
	if( (ret = dlsym(mysql.handle, name)) == NULL ) {
		pline("%s", nh_dlerror());
		return NULL;
	}
	return ret;
}

#define MAX_LIBNAME_LEN 64
static const char *libname( const char *name ) {
	assert(name != NULL);
	static const char *libsuffix;
#if defined(__APPLE__) && defined(__MACH__)
	libsuffix = "dylib";
#else
	libsuffix = "so";
#endif
	static char buf[MAX_LIBNAME_LEN];
	size_t len = snprintf(buf, MAX_LIBNAME_LEN, "lib%s.%s", name, libsuffix);
	assert(len < MAX_LIBNAME_LEN);
	return buf;
}

mysql_t *_get_mysql() {
	static mysql_t _mysql = {
		.handle = NULL
	};
	return &_mysql;
}

bool mysql_library_shutdown() {
	if( mysql.handle != NULL ) {
		if( dlclose(mysql.handle) != 0 ) {
			pline("%s", nh_dlerror());
			return false;
		} else {
			mysql.handle = NULL;
		}
	}
	return true;
}

bool mysql_library_startup() {
	if( mysql_library_disabled ) return true;
	if( mysql.handle != NULL ) return true;

	//Dynamically load MYSQL library
	if( (mysql.handle = dlopen(libname("mysqlclient"), RTLD_LAZY)) == NULL ) {
		pline("Achievement system unavailabe: %s", nh_dlerror());
		mysql_library_disabled = true;
		return false;
	}
	if( (mysql.init = mysql_function("mysql_init")) == NULL ||
	    (mysql.real_connect = mysql_function("mysql_real_connect")) == NULL ||
	    (mysql.real_query = mysql_function("mysql_real_query")) == NULL ||
	    (mysql.use_result = mysql_function("mysql_use_result")) == NULL ||
	    (mysql.free_result = mysql_function("mysql_free_result")) == NULL ||
	    (mysql.fetch_row = mysql_function("mysql_fetch_row")) == NULL ||
	    (mysql.error = mysql_function("mysql_error")) == NULL ||
	    (mysql.ping = mysql_function("mysql_ping")) == NULL ||
	    (mysql.options = mysql_function("mysql_options")) == NULL ||
	    (mysql.num_rows = mysql_function("mysql_num_rows")) == NULL ||
		(mysql.close = mysql_function("mysql_close")) == NULL ||
		(mysql.real_escape_string = mysql_function("mysql_real_escape_string")) == NULL ) {
		mysql_library_disabled = true;
		mysql_library_shutdown();
		return false;
	}

	return true;
}

bool mysql_library_available() {
	return !mysql_library_disabled;
}

char *mysql_library_escape_string( MYSQL *db, const char *str ) {
	char *to;
	size_t len = strlen(str);
	if( (to = malloc(len * 2 + 1)) == NULL ) panic("malloc: %s", strerror(errno));
	mysql.real_escape_string(db, to, str, len);
	return to;
}
