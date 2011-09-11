#include <dlfcn.h>
#include <stdbool.h>

#include <mysql.h>

#include "achieve.h"
#include "hack.h"

struct {
	void *handle;
	MYSQL db;

	// MySQL library functions
	MYSQL *(*init)( MYSQL * );
	MYSQL *(*real_connect)( MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, const char *unix_socket, unsigned long client_flag );
	int (*real_query)( MYSQL *mysql, const char *stmt_str, unsigned long length );
	MYSQL_RES *(*use_result)( MYSQL *mysql );
	void (*free_result)( MYSQL_RES *result );
	MYSQL_ROW (*fetch_row)( MYSQL_RES *result );
} mysql = {
	.handle = NULL
};

static char *nh_dlerror() {
	char *err = dlerror();
	return err == NULL ? "dlopen failed and no error message" : err;
}

static void dl_impossible() {
	impossible("%s\n", nh_dlerror());
}

static void *mysql_function( const char *name ) {
	if( mysql.handle == NULL ) impossible("Achievement system has not been initialized\n");
	void *ret;
	if( (ret = dlsym(mysql.handle, name)) == NULL ) dl_impossible();
	return ret;
}

#if defined(__APPLE__) && defined(__MACH__)
#define libname(s) (s ".dylib")
#else
#define libname(s) (s ".so")
#endif

void achievement_system_startup(){
	if( mysql.handle != NULL ) return;
	if( (mysql.handle = dlopen(libname("libmysqlclient"), RTLD_LAZY)) == NULL ) {
		pline("Achievement system unavailabe: %s\n", nh_dlerror());
		return;
	}
	mysql.init = mysql_function("mysql_init");
	mysql.real_connect = mysql_function("mysql_real_connect");
	mysql.real_query = mysql_function("mysql_real_query");
	mysql.use_result = mysql_function("mysql_use_result");
	mysql.free_result = mysql_function("mysql_free_result");
	mysql.fetch_row = mysql_function("mysql_fetch_row");
	mysql.init(&mysql.db);
}

#undef libname

void achievement_system_shutdown() {
	if( mysql.handle != NULL ) {
		if( dlclose(mysql.handle) != 0 ) {
			dl_impossible();
		} else {
			mysql.handle = NULL;
		}
	}
}

//ret 1 on sucess, 0 on failure
int add_achievement_progress(int achievement_id, int add_progress_count){
	pline("DEBUG: add_achievement_progress(%i, %i)\n", achievement_id, add_progress_count);
	achievement_system_startup();
	if( mysql.handle == NULL ) return 0;
	int pre_achievement_progress = get_achievement_progress(achievement_id);
	int max_achievement_progress = get_achievement_max_progress(achievement_id);
	if(pre_achievement_progress < max_achievement_progress){ //user still needs achievement
		if(pre_achievement_progress + add_progress_count >= max_achievement_progress){ //Achievement fully achieved!
			if(push_achievement_progress(achievement_id, max_achievement_progress)){ //floor the value to max_progress
				pline("Congratulations! You've earned the achievement: %s\n", get_achievement_name(achievement_id));
				return ACHIEVEMENT_PUSH_SUCCESS;
			}
			else{
				pline("Er, oops. You got an achievement, but it can't be recorded.\n");
				return ACHIEVEMENT_PUSH_FAILURE;
			}
		}
		else{ //user stills needs achievement, but isn't quite there yet
			if(push_achievement_progress(achievement_id, pre_achievement_progress+add_progress_count)){
				return ACHIEVEMENT_PUSH_SUCCESS;
			}
			else{
				return ACHIEVEMENT_PUSH_FAILURE;
			}
		}
	}
	return 1;
}

int get_achievement_progress(int achievement_id){
	pline("DEBUG: get_achievement_progress(%i)\n", achievement_id);
	return 0;
}

int get_achievement_max_progress(int achievement_id){
	return 1;
}

//REQUIRES timeout on non-success!!!!!!!
int push_achievement_progress(int achievement_id, int updated_progress_count){
	return 1;
}

char * get_achievement_name(int achievement_id){
	return "TEST DATA";
}
