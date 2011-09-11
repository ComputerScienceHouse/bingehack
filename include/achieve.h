#ifndef ACHIEVE_H
#define ACHIEVE_H

const static int GAME_ID = 1;

const static int AID_POTTER = 1;
const static int AID_MAIL = 1;

const static int ONE_TIME_ACHIEVEMENT = 1;

const static int ACHIEVEMENT_PUSH_SUCCESS = 1;
const static int ACHIEVEMENT_PUSH_FAILURE = 0;

void achievement_system_startup();
__attribute__((destructor)) void achievement_system_shutdown();

//ret 1 on sucess, 0 on failure
int add_achievement_progress(int achievement_id, int add_progress_count);

int get_achievement_progress(int achievement_id);

int get_achievement_max_progress(int achievement_id);

int push_achievement_progress(int achievement_id, int updated_progress_count);

char * get_achievement_name(int achievement_id);

#endif
