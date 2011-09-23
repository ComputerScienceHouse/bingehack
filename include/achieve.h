#ifndef ACHIEVE_H
#define ACHIEVE_H

const static int ACHIEVEMENT_GAME_ID = 1;

// TBI == To Be Implemented
enum {
	/* General */
	AID_POTTER_CHAT = 1,
	AID_MAIL_RECEIVE = 2,
	AID_ASCEND_THRICE = 6, //TBI
	AID_VAULT_LIE = 7,
	AID_SOKOBAN_DESTROY_BOULDER = 8, //TBI
	AID_ENGRAVE_ELBERETH = 9,
	AID_POLYMORPH_SELF = 11, //TBI
	AID_KILL_DEV_GHOST = 12, //TBI
	AID_KILLED_HELPLESS = 13, //TBI
	AID_KILLED_BY_ANTS = 14, //TBI
	AID_WALK_5K = 17, //TBI
	AID_WALK_10K = 18, //TBI
	AID_CRASH = 19,
	AID_UNCARING_GOD = 22, //TBI
	AID_DIG_FOR_VICTORY = 26, //TBI
	AID_RUBBING_IT_IN = 27, //TBI
	AID_RESURRECT_PLAYERS = 29, //TBI
	AID_SKIN_OF_TEETH = 30, //TBI
	AID_KOPS_FIGHT = 31, //TBI
	AID_DJINN_THREE_WISHES = 32, //TBI
	AID_BLIND_GAZER_WITH_PIE = 42, //TBI
	AID_EXTINCTION = 43, //TBI
	AID_GET_ALL_AMULETS = 44, //TBI
	AID_ENCHANT_HIGH = 45, //TBI
	AID_TIN_OF_RODNEY = 46, //TBI
	AID_CANNIBALISM = 47, //TBI
	AID_STONE_A_GOLEM = 48, //TBI
	AID_EAT_ALL_ARTIFICIAL_MEAT = 49, //TBI
	AID_SURVIVE_GOD_DISINTEGRATION = 50, //TBI
	AID_PUNISHMENT = 51, //TBI
	AID_TERRIBLE_AC = 52, //TBI
	AID_AWESOME_AC = 53, //TBI
	AID_INTRINSICS_RESISTANCE = 54, //TBI
	AID_INTRINSICS_NINJA = 55, //TBI
	AID_TRIVIAL_LEVEL_UP = 56, //TBI
	AID_CONVERT_ALTAR = 57, //TBI
	AID_KILL_GUARANTEED_DEMONS = 58, //TBI
	AID_MIRROR_SMASH_KILL = 59, //TBI
	AID_MASTER_CASTER = 73, //TBI
	AID_ARTIFACT_BLAST = 74, //TBI
	AID_TAME_FOOCUBUS = 75, //TBI
	AID_KILL_DEMOGORGON = 76, //TBI
	AID_RUBBER_CHICKEN = 78, //TBI
	AID_ALCHEMIZE_GAIN_LEVEL = 79, //TBI
	AID_ALCHEMIZE_EXPLOSION = 80, //TBI
	AID_ZAP_RODNEY_WITH_DEATH = 81, //TBI
	AID_WREST_FROM_RECHARGED_WOW = 82, //TBI
	AID_BLIND_RIDER_WITH_CAMERA = 83, //TBI
	AID_ASCENSION_KIT = 84, //TBI
	AID_FALL_DOWN_STAIRS = 85, //TBI
	AID_BREAK_URANIUM_WAND = 86, //TBI
	/* Game progress */
	AID_START_QUEST = 10,
	AID_KILL_VLAD = 15,
	AID_GET_BOTD = 77,
	AID_FIND_VIBE_SQUARE = 16,
	AID_GET_AOY = 4,
	AID_ASCEND = 5,
	/* Quest artifacts ("As <role>, <do something> using <your quest artifact>") */
	AID_USE_ARTIFACT_ARC = 60, //TBI
	AID_USE_ARTIFACT_BAR = 61, //TBI
	AID_USE_ARTIFACT_CAV = 62, //TBI
	AID_USE_ARTIFACT_HEA = 63, //TBI
	AID_USE_ARTIFACT_KNI = 64, //TBI
	AID_USE_ARTIFACT_MON = 65, //TBI
	AID_USE_ARTIFACT_PRI = 66, //TBI
	AID_USE_ARTIFACT_RAN = 67, //TBI
	AID_USE_ARTIFACT_ROG = 68, //TBI
	AID_USE_ARTIFACT_SAM = 69, //TBI
	AID_USE_ARTIFACT_TOU = 70, //TBI
	AID_USE_ARTIFACT_VAL = 71, //TBI
	AID_USE_ARTIFACT_WIZ = 72, //TBI
	/* Conducts ("End a game at XL15 or higher with <conduct> intact") */
	AID_CONDUCT_FOODLESS = 33, //TBI
	AID_CONDUCT_VEGETARIAN = 34, /* Obviously includes vegan */ //TBI
	AID_CONDUCT_ATHEIST = 35, //TBI
	AID_CONDUCT_PACIFIST = 36, //TBI
	AID_CONDUCT_WEAPONLESS = 37, //TBI
	AID_CONDUCT_ILLITERATE = 38, //TBI
	AID_CONDUCT_NOPOLYMORPH = 39, /* Both polyless AND polyselfless! */ //TBI
	AID_CONDUCT_GENOCIDELESS = 40, //TBI
	AID_CONDUCT_WISHLESS = 41, //TBI
	/* Achievements not awarded in-game */
	AID_CODE_COMMIT = 23,
	AID_BINGE_PARTICIPATE = 24,
	AID_BINGE_FULL_PARTICIPATE = 25
};

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
