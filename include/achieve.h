#ifndef ACHIEVE_H
#define ACHIEVE_H

const static int GAME_ID = 1;

const static int AID_POTTER = 1;
const static int AID_MAIL = 2;
const static int AID_VAULT = 7;
const static int AID_ELBERETH = 9;
const static int AID_CRASH = 19;

const static int ACHIEVEMENT_PUSH_SUCCESS = 1;
const static int ACHIEVEMENT_PUSH_FAILURE = 0;

bool achievement_system_startup();
void achievement_system_shutdown();

// Return 1 on success, 0 on failure
int add_achievement_progress(int achievement_id, int add_progress_count);

// As above
int award_achievement(int achievement_id);

int get_achievement_progress(int achievement_id);

int get_achievement_max_progress(int achievement_id);

int push_achievement_progress(int achievement_id, int updated_progress_count);

char * get_achievement_name(int achievement_id);

void disable_achievements();

int user_exists();

int register_user();

int check_db_connection();
#endif
