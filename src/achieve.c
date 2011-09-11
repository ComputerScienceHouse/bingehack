#include "achieve.h"
#include "hack.h"

#ifdef ACHIEVEMENTS
#include <mysql/mysql.h>
MYSQL mysql;
#endif

int achievement_system_online = 0;

void achievement_system_startup(){
#ifdef ACHIEVEMENTS
	if(!achievement_system_online){
		mysql_init(&mysql);
		achievement_system_online = 1;
	}
#endif
}

//ret 1 on sucess, 0 on failure
int add_achievement_progress(int achievement_id, int add_progress_count){
#ifdef ACHIEVEMENTS
	pline("DEBUG: add_achievement_progress(%i, %i)", achievement_id, add_progress_count);
	achievement_system_startup();
	int pre_achievement_progress = get_achievement_progress(achievement_id);
	int max_achievement_progress = get_achievement_max_progress(achievement_id);
	if(pre_achievement_progress < max_achievement_progress){ //user still needs achievement
		if(pre_achievement_progress + add_progress_count >= max_achievement_progress){ //Achievement fully achieved!
			if(push_achievement_progress(achievement_id, max_achievement_progress)){ //floor the value to max_progress
				pline("Congratulations! You've earned the achievement: %s", get_achievement_name(achievement_id));
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
	return 1;
#else
	return 1;
#endif
}

int get_achievement_progress(int achievement_id){
#ifdef ACHIEVEMENTS
	pline("DEBUG: get_achievement_progress(%i)", achievement_id);
	return 0;
#else
	return 0;
#endif
}

int get_achievement_max_progress(int achievement_id){
#ifdef ACHIEVEMENTS
	return 1;
#else
	return 1;
#endif
}

//REQUIRES timeout on non-success!!!!!!!
int push_achievement_progress(int achievement_id, int updated_progress_count){
#ifdef ACHIEVEMENTS
	return 1;
#else
	return 1;
#endif
}

char * get_achievement_name(int achievement_id){
#ifdef ACHIEVEMENTS
	return "TEST DATA";
#else
	return "TEST DATA";
#endif
}
