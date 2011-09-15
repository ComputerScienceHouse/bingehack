#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <mysql.h>
#include <libconfig.h>

#include "configfile.h"
#include "achieve.h"
#include "hack.h"
#include <ctype.h>

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
	const char *(*error)( MYSQL *mysql );
	int (*ping)( MYSQL *mysql );
	int (*options)( MYSQL *mysql, enum mysql_option option, const void *arg );
	my_ulonglong (*num_rows)( MYSQL_RES *result );
} mysql = {
	.handle = NULL
};

const int ACHIEVEMENT_DEBUG = 0; 
int achievement_system_disabled = 0;

static char *nh_dlerror() {
	char *err = dlerror();
	return err == NULL ? "dylib routine failed and no error message" : err;
}

static void *mysql_function( const char *name ) {
	if( mysql.handle == NULL ) panic("Achievement system has not been initialized");
	void *ret;
	if( (ret = dlsym(mysql.handle, name)) == NULL ) panic("%s", nh_dlerror());
	return ret;
}

static bool config_get_string( const char *path, const char **str ) {
	if( config_lookup_string(config, path, str) == CONFIG_FALSE ) {
		pline("Config setting %s not found or wrong type", path);
		return false;
	}
	return true;
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

bool achievement_system_startup() {
	if( mysql.handle != NULL ) return true;

	//read database configuration from file
	const char *db_user, *db_pass, *db_db, *db_server;
	if( !config_get_string("achievements.mysql.username", &db_user) ||
	    !config_get_string("achievements.mysql.password", &db_pass) ||
	    !config_get_string("achievements.mysql.database", &db_db) ||
	    !config_get_string("achievements.mysql.server",   &db_server) ) return false;

	//Dynamically load MYSQL library
	if( (mysql.handle = dlopen(libname("mysqlclient"), RTLD_LAZY)) == NULL ) {
		pline("Achievement system unavailabe: %s", nh_dlerror());
		return false;
	}
	mysql.init = mysql_function("mysql_init");
	mysql.real_connect = mysql_function("mysql_real_connect");
	mysql.real_query = mysql_function("mysql_real_query");
	mysql.use_result = mysql_function("mysql_use_result");
	mysql.free_result = mysql_function("mysql_free_result");
	mysql.fetch_row = mysql_function("mysql_fetch_row");
	mysql.error = mysql_function("mysql_error");
	mysql.ping = mysql_function("mysql_ping");
	mysql.options = mysql_function("mysql_options");
	mysql.num_rows = mysql_function("mysql_num_rows");
	mysql.init(&mysql.db);
	
	//Respond in 3 seconds or assume db server is unreachable
	int timeout_seconds = 3;
	int ret = mysql.options(&mysql.db, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_seconds);
	assert(ret == 0);

	if(mysql.real_connect(&mysql.db, db_server, db_user, db_pass, db_db, 0, NULL, 0) == NULL){
		pline("Can't connect to achievements database. Database error: %s", mysql.error(&mysql.db));
		disable_achievements();
		return false;
	}

	return true;
}

void achievement_system_shutdown() {
	if( mysql.handle != NULL ) {
		if( dlclose(mysql.handle) != 0 ) {
			panic("%s", nh_dlerror());
		} else {
			mysql.handle = NULL;
		}
	}
}

//ret 1 on sucess, 0 on failure
int add_achievement_progress(int achievement_id, int add_progress_count){
	if(achievement_system_disabled){ return ACHIEVEMENT_PUSH_FAILURE; }
	if(ACHIEVEMENT_DEBUG){pline("DEBUG: add_achievement_progress(%i, %i)", achievement_id, add_progress_count);}
	
	if( !achievement_system_startup() ) return ACHIEVEMENT_PUSH_FAILURE;

	//Check if user exists
	if(!user_exists()){
		register_user();
	}
	//Calculate user's completion on this achievement
	int pre_achievement_progress = get_achievement_progress(achievement_id);
	int max_achievement_progress = get_achievement_max_progress(achievement_id);
	if(ACHIEVEMENT_DEBUG){pline("DEBUG: get_achievement_max_progress(%i)=%i", achievement_id, max_achievement_progress);}

	if(pre_achievement_progress < max_achievement_progress){ //user still needs achievement
		if(pre_achievement_progress + add_progress_count >= max_achievement_progress){ //Achievement fully achieved!
			if(push_achievement_progress(achievement_id, max_achievement_progress)){ //floor the value to max_progress
				char * achievement_name = get_achievement_name(achievement_id);
				pline("Congratulations! You've earned the achievement: %s", achievement_name);
				free(achievement_name);
				return ACHIEVEMENT_PUSH_SUCCESS;
			}
			else{
				pline("Er, oops. You got an achievement, but it can't be recorded.");
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
	return ACHIEVEMENT_PUSH_SUCCESS;
}

int get_achievement_progress(int achievement_id){
	if( achievement_system_disabled ) return -1;

	char* query;
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	char* str_progress;
	int achievement_progress = -1;

	if( asprintf(&query, "SELECT `achievement_progress`.`progress` FROM `achievement_progress` JOIN `users_in_apps` ON `users_in_apps`.`user_id` = `achievement_progress`.`user_id` where app_username = '%s' and app_id = %i and achievement_id = %i;", plname, GAME_ID, achievement_id) == -1 ) panic("asprintf: %s", strerror(errno));
	if( mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0 ) goto fail;
	free(query);
	if( (res = mysql.use_result(&mysql.db)) == NULL ) goto fail;
	if( (row = mysql.fetch_row(res)) == NULL ) {
		if( mysql.num_rows(res) == 0 ) {
			achievement_progress = 0;
			goto out;
		} else {
			goto fail;
		}
	} else {
		if( asprintf(&str_progress, "%s", row[0]) == -1 ) panic("asprintf: %s", strerror(errno));
		achievement_progress = atoi(str_progress);
		free(str_progress);
	}

	if(ACHIEVEMENT_DEBUG) pline("DEBUG: get_achievement_progress(%i)=%i", achievement_id, achievement_progress);
	
	row = mysql.fetch_row(res);
	assert(row == NULL);
	goto out;

fail:
	pline("Error in get_achievement_progress(%i) (Error: %s)", achievement_id, mysql.error(&mysql.db));
	disable_achievements();
	achievement_progress = -1;

out:
	if( res != NULL ) mysql.free_result(res);
	return achievement_progress;
}

int get_achievement_max_progress(int achievement_id){
	if( achievement_system_disabled ) return -1;

	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	char* query;
	char* str_max_progress;
	int max_progress=0;

	if( asprintf(&query, "SELECT `achievements`.`progress_max` FROM `achievements` WHERE id = %i ;", achievement_id) == -1 ) panic("asprintf: %s", strerror(errno));
	if( mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0 ) goto fail;
	free(query);
	if( (res = mysql.use_result(&mysql.db)) == NULL ) goto fail;

	if( (row = mysql.fetch_row(res)) == NULL ) {
		if( mysql.num_rows(res) == 0 ) panic("Error in get_achievement_max_progress(%i): Requested achievement does not exist.", achievement_id);
		goto fail;
	}
	if( asprintf(&str_max_progress, "%s", row[0]) == -1 ) panic("asprintf: %s", strerror(errno));
	max_progress = atoi(str_max_progress);

	free(str_max_progress);

	row = mysql.fetch_row(res);
	assert(row == NULL);
	goto out;

fail:
	pline("Error in get_achievement_max_progress(%i) (Error: %s)", achievement_id, mysql.error(&mysql.db));
	disable_achievements();
	max_progress = -1;

out:
	if( res != NULL ) mysql.free_result(res);
	return max_progress;
}

//REQUIRES timeout on non-success!!!!!!!
int push_achievement_progress(int achievement_id, int updated_progress_count){
	char* query;

	int progress = get_achievement_progress(achievement_id);
	if( progress == -1 ) return -1;
	if( progress == 0) { //no previous progress on achievement, INSERT time
		if( asprintf(&query, "INSERT INTO `achievement_progress`(`user_id`, `achievement_id`, `progress`) VALUES ((select user_id from `users_in_apps` where app_id=1 and app_username='%s'), %i, %i);", plname, achievement_id, updated_progress_count) == -1 ) panic("asprintf: %s", strerror(errno));
		if(mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0){
			pline("Real_query failed in push_achievement_progress: %s", mysql.error(&mysql.db));
			disable_achievements();
		}
		free(query);
		return 1;
	} else { //TODO: Implement Multi-part achievements (aka UPDATE)
		return 0;
	}
}

// It is the caller's responsibility to free() the return value of this function.
char *get_achievement_name( int achievement_id ){
		MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	char* query;
	char* str_name;

	if( asprintf(&query, "SELECT `achievements`.`title` FROM `achievements` WHERE id = %i ;", achievement_id) == -1 ) panic("asprintf: %s", strerror(errno));
	if( mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0 ) goto fail;
	free(query);

	if( (res = mysql.use_result(&mysql.db)) == NULL ) goto fail;
	if( (row = mysql.fetch_row(res)) == NULL ) {
		if( mysql.num_rows(res) == 0 ) panic("Attempt to call non-existant achievement_id %i", achievement_id);
		goto fail;
	}
	if( (str_name = strdup(row[0])) == NULL ) panic("strdup: %s", strerror(errno));
	if(ACHIEVEMENT_DEBUG) pline("DEBUG: get_achievements_name(%i)='%s'", achievement_id, str_name);

	row = mysql.fetch_row(res);
	assert(row == NULL);

	goto out;

fail:
	pline("Error in get_achievement_name(%i) (Error: %s)", achievement_id, mysql.error(&mysql.db));
	disable_achievements();
	str_name = strdup("");
	if( str_name == NULL ) panic("strdup: %s", strerror(errno));

out:
	if( res != NULL ) mysql.free_result(res);
	return str_name;
}

void disable_achievements(){
	pline("Disabling the achievements system for the duration of this session.");
	achievement_system_disabled = 1;
}

//TODO Odd unregistered case where app_username is already taken
int register_user(){
	pline("Hey! Listen! You're not registered in the achievements database.");
	pline("If you don't have a CSH account, you can probably leave this blank.");
	char str[BUFSZ];
	getlin("Enter CSH username to associate with this account:", str);
	if(isspace(str[0]) || str[0] == '\033' || str[0] == '\n' || str[0] == 0){
		memset(str, 0, BUFSZ);
		strcpy(str,plname);
	}
	
	char *query;
	if( asprintf(&query, "INSERT INTO `users` (`username`) VALUES ('%s')", str ) == -1) panic("asprintf: %s", strerror(errno));
	
	if(mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0){
		pline("Real_query failed in register_user: %s", mysql.error(&mysql.db));
		disable_achievements();
	}
	free(query);
	
	if( asprintf(&query, "INSERT INTO `users_in_apps` VALUES ((SELECT `id` FROM `users` WHERE `users`.`username` = '%s'), %i, '%s')", str, GAME_ID , plname ) == -1) panic("asprintf: %s", strerror(errno));

	if(mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0){
		pline("Real_query failed in register_user: %s", mysql.error(&mysql.db));
		disable_achievements();
	}
	free(query);

	return 1;
}

int user_exists(){
	MYSQL_RES *res = NULL;
	MYSQL_ROW row;
	char* query;
	int user_exists = 0;

	if( asprintf(&query, "SELECT `app_username` FROM `users_in_apps` WHERE app_username = '%s' AND app_id = %i ;", plname, GAME_ID) == -1 ) panic("asprintf: %s", strerror(errno));
	if( mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0 ){
		goto fail;
	}
	free(query);

	if((res = mysql.use_result(&mysql.db)) == NULL){
		disable_achievements();
	}
	else{
		if( (row = mysql.fetch_row(res)) == NULL ) {
			if( mysql.num_rows(res) == 0 ){ //Not a user of nethack
				user_exists = 0;
			}
		}
		else{
			row = mysql.fetch_row(res);
			assert(row == NULL);
			user_exists = 1;
		}
	}
	goto out;

fail:
	pline("Error in user_exists() (Error: %s)", mysql.error(&mysql.db));
	disable_achievements();
out:
	if( res != NULL ) mysql.free_result(res);

	return user_exists;
}
