#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <string.h>

#include <mysql.h>
#include <libconfig.h>

#include "configfile.h"
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
	unsigned long (*fetch_lengths) (MYSQL_RES *result);
	const char *(*error) (MYSQL *mysql);
	int (*ping) (MYSQL *mysql);
	int (*options) (MYSQL *mysql, enum mysql_option option, const void *arg);
} mysql = {
	.handle = NULL
};

const int ACHIEVEMENT_DEBUG = 0; 
int started = 0; 
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

#if defined(__APPLE__) && defined(__MACH__)
#define libname(s) (s ".dylib")
#else
#define libname(s) (s ".so")
#endif

void achievement_system_startup(){
	
	//read database configuration from file
	const char *db_user, *db_pass, *db_db, *db_server;
	if( !config_get_string("achievements.mysql.username", &db_user)   ) return;
	if( !config_get_string("achievements.mysql.password", &db_pass)   ) return;
	if( !config_get_string("achievements.mysql.database", &db_db)	  ) return;
	if( !config_get_string("achievements.mysql.server",   &db_server) ) return; 


	//Dynamically load MYSQL library	
	if( mysql.handle != NULL ) return;
	if( (mysql.handle = dlopen(libname("libmysqlclient"), RTLD_LAZY)) == NULL ) {
		pline("Achievement system unavailabe: %s", nh_dlerror());
		return;
	}
	mysql.init = mysql_function("mysql_init");
	mysql.real_connect = mysql_function("mysql_real_connect");
	mysql.real_query = mysql_function("mysql_real_query");
	mysql.use_result = mysql_function("mysql_use_result");
	mysql.free_result = mysql_function("mysql_free_result");
	mysql.fetch_lengths = mysql_function("mysql_fetch_lengths");
	mysql.fetch_row = mysql_function("mysql_fetch_row");
	mysql.error = mysql_function("mysql_error");
	mysql.ping = mysql_function("mysql_ping");
	mysql.options = mysql_function("mysql_options");
	mysql.init(&mysql.db);
	
	//Respond in 3 seconds or assume db server is unreachable
	int timeout_seconds = 3;
	mysql.options(&mysql.db, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_seconds);

	if(mysql.real_connect(&mysql.db, db_server, db_user, db_pass, db_db, 0, NULL, 0) == NULL){
		pline("Couldn't connect to db server for achievements. Specific error: %s", mysql.error(&mysql.db));
		disable_achievements();
		started = 0;
	}
	started = 1;
}

#undef libname

void achievement_system_shutdown() {
	if( mysql.handle != NULL ) {
		if( dlclose(mysql.handle) != 0 ) {
			impossible("%s", nh_dlerror());
		} else {
			mysql.handle = NULL;
		}
	}
}

//ret 1 on sucess, 0 on failure
int add_achievement_progress(int achievement_id, int add_progress_count){
	if(achievement_system_disabled){ return ACHIEVEMENT_PUSH_FAILURE; }
	if(ACHIEVEMENT_DEBUG){pline("DEBUG: add_achievement_progress(%i, %i)", achievement_id, add_progress_count);}
	
	if(!started){achievement_system_startup();}
	if( mysql.handle == NULL ) return ACHIEVEMENT_PUSH_FAILURE;
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
	char* query;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* str_progress;
	int achievement_progress=0;

	asprintf(&query, "SELECT `achievement_progress`.`progress` FROM `achievement_progress` JOIN `users_in_apps` ON `users_in_apps`.`user_id` = `achievement_progress`.`user_id` where app_username = '%s' and app_id = %i and achievement_id = %i;", plname, GAME_ID, achievement_id);
	if(mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0){
		pline("Error in get_achievement_progress(%i) (Error: %s)", achievement_id, mysql.error(&mysql.db));
		disable_achievements();
	}
	free(query);
	if(!achievement_system_disabled){res = mysql.use_result(&mysql.db);}
	if(res == NULL){
		pline("Error in get_achievement_progress(%i) (Error: %s)", achievement_id, mysql.error(&mysql.db));
		disable_achievements();
	}
	else{
		row = mysql.fetch_row(res);
		if( mysql.fetch_lengths(res) == 0) {	//NO ACHIEVEMENT PROGRESS
			achievement_progress = 0;
		}
		else{
			asprintf(&str_progress, "%s", row[0]);
			achievement_progress = atoi(str_progress);
			free(str_progress);
		}
	
		if(ACHIEVEMENT_DEBUG){pline("DEBUG: get_achievement_progress(%i)=%i", achievement_id, achievement_progress);}
		
		while( mysql.fetch_row(res) != NULL){} //keep fapping
		mysql.free_result(res);
	}
	return achievement_progress;
}

int get_achievement_max_progress(int achievement_id){
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* query;
	char* str_max_progress;
	int max_progress;

	asprintf(&query, "SELECT `achievements`.`progress_max` FROM `achievements` WHERE id = %i ;", achievement_id);
	if(mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0){
		panic("Real_query failed in get_achievement_max_progress: %s", mysql.error(&mysql.db));
	}
	free(query);
	res = mysql.use_result(&mysql.db);
	row = mysql.fetch_row(res);
	if( mysql.fetch_lengths(res) == 0) {	//ACHIEVEMENT DOES NOT EXIST
		panic("Error in get_achievement_max_progress(%i): Requested achievement does not exist.");
	}

	asprintf(&str_max_progress, "%s", row[0]);
	max_progress = atoi(str_max_progress);

	free(str_max_progress);

	while( mysql.fetch_row(res) != NULL){} //keep fapping
	mysql.free_result(res);
	
	return max_progress;
}

//REQUIRES timeout on non-success!!!!!!!
int push_achievement_progress(int achievement_id, int updated_progress_count){
	char* query;
	
	if(get_achievement_progress(achievement_id) == 0){//no previous progress on achievement, INSERT time
		asprintf(&query, "INSERT INTO `achievement_progress`(`user_id`, `achievement_id`, `progress`) VALUES ((select user_id from `users_in_apps` where app_id=1 and app_username='%s'), %i, %i);", plname, achievement_id, updated_progress_count);
		if(mysql.real_query(&mysql.db, query, (unsigned int) strlen(query)) != 0){
		       pline("Real_query failed in push_achievement_progress: %s", mysql.error(&mysql.db));
			disable_achievements();
		}
		free(query);
		return 1;
	}
	else{	//TODO: Implement Multi-part achievements (aka UPDATE)
		return 0;
	}
}

char * get_achievement_name(int achievement_id){
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* query;
	char* str_name;

	asprintf(&query, "SELECT `achievements`.`title` FROM `achievements` WHERE id = %i ;", achievement_id);
	mysql.real_query(&mysql.db, query, (unsigned int) strlen(query));
	free(query);

	res = mysql.use_result(&mysql.db);
	if(res == NULL){
		panic("Error in get_achievement_name(%i) (Error: %s)", achievement_id, mysql.error(&mysql.db));
	}
	
	row = mysql.fetch_row(res);
	if(mysql.fetch_lengths(res) == 0){
		panic("Attempt to call non-existant achievement_id %i", achievement_id);
	}
	
	str_name = strdup(row[0]);
	if(ACHIEVEMENT_DEBUG){pline("DEBUG: get_achievements_name(%i)='%s'", achievement_id, str_name);}

	while( mysql.fetch_row(res) != NULL){} //keep fapping
	mysql.free_result(res);

	return str_name;
}

void disable_achievements(){
	pline("Disabling the achievements system for the duration of this session.");
	achievement_system_disabled=1;
}
