#ifndef ACHIEVE_H
#define ACHIEVE_H

const static int ACHIEVEMENT_APP_ID = 1;

// TBI == To Be Implemented
enum {
	/* General */
	AID_POTTER_CHAT = 1,
	AID_MAIL_RECEIVE = 2,
	AID_ASCEND_THRICE = 6,
	AID_VAULT_LIE = 7,
	AID_SOKOBAN_DESTROY_BOULDER = 8,
	AID_ENGRAVE_ELBERETH = 9,
	AID_POLYMORPH_SELF = 11,
	AID_KILL_GHOSTS = 12,
	AID_KILLED_HELPLESS = 13,
	AID_KILLED_BY_ANT = 14,
	AID_WALK_5K = 17,
	AID_WALK_10K = 18,
	AID_CRASH = 19,
	AID_UNCARING_GOD = 22,
	AID_DIG_FOR_VICTORY = 26,
	AID_RUBBING_IT_IN = 27, //TBI
	AID_RESURRECT_PLAYERS = 29,
	AID_SKIN_OF_TEETH = 30,
	AID_FOUGHT_THE_LAW = 31,
	AID_DJINN_WISHES = 32,
	AID_BLIND_GAZER_WITH_PIE = 42,
	AID_GENOCIDES = 43,
	AID_GET_ALL_AMULETS = 44,
	AID_ENCHANT_HIGH = 45,
	AID_TIN_OF_RODNEY = 46,
	AID_CANNIBALISM = 47,
	AID_STONE_A_GOLEM = 48,
	AID_EAT_HUGE_CHUNK_OF_MEAT = 49,
	AID_SURVIVE_GOD_DISINTEGRATION = 50,
	AID_PUNISHMENT = 51,
	AID_TERRIBLE_AC = 52,
	AID_AWESOME_AC = 53,
	AID_INTRINSICS_RESISTANCE = 54, //TBI
	AID_INTRINSICS_NINJA = 55, //TBI
	AID_TRIVIAL_LEVEL_UP = 56,
	AID_CONVERT_ALTAR = 57,
	AID_KILL_GUARANTEED_DEMONS = 58,
	AID_MIRROR_SMASH_KILL = 59,
	AID_MASTER_CASTER = 73,
	AID_ARTIFACT_BLAST = 74,
	AID_TAME_FOOCUBUS = 75,
	AID_KILL_DEMOGORGON = 76,
	AID_RUBBER_CHICKEN = 78,
	AID_ALCHEMIZE_GAIN_LEVEL = 79,
	AID_ALCHEMIZE_EXPLOSION = 80,
	AID_ZAP_RODNEY_WITH_DEATH = 81,
	AID_WREST_FROM_RECHARGED_WOW = 82,
	AID_BLIND_RIDER_WITH_CAMERA = 83,
	AID_ASCENSION_KIT = 84,
	AID_FALL_DOWN_STAIRS = 85,
	AID_BREAK_URANIUM_WAND = 86, //TBI
	AID_KILL_GRID_BUGS = 88,
	AID_KILL_UNDEAD = 89,
	AID_KILL_ORCS = 91,
	AID_KILL_WOODCHUCK = 92,
	AID_KILL_DRAGONS = 93,
	AID_SCHROEDINGERS_CAT = 94,
	/* Game progress */
	AID_START_QUEST = 10,
	AID_GET_BELL = 87,
	AID_GET_CANDELABRUM = 15,
	AID_ENTER_GEHENNOM = 90,
	AID_GET_BOOK = 77,
	AID_FIND_VIBE_SQUARE = 16,
	AID_GET_AMULET = 4,
	AID_ASCEND = 5,
	/* Quest artifacts ("As <role>, <do something> using <your quest artifact>") */
	AID_USE_ARTIFACT_ARC = 60,
	AID_USE_ARTIFACT_BAR = 61,
	AID_USE_ARTIFACT_CAV = 62,
	AID_USE_ARTIFACT_HEA = 63,
	AID_USE_ARTIFACT_KNI = 64,
	AID_USE_ARTIFACT_MON = 65,
	AID_USE_ARTIFACT_PRI = 66,
	AID_USE_ARTIFACT_RAN = 67,
	AID_USE_ARTIFACT_ROG = 68,
	AID_USE_ARTIFACT_SAM = 69,
	AID_USE_ARTIFACT_TOU = 70,
	AID_USE_ARTIFACT_VAL = 71,
	AID_USE_ARTIFACT_WIZ = 72,
	/* Conducts ("End a game at XL15 or higher with <conduct> intact") */
	AID_CONDUCT_FOODLESS = 33,
	AID_CONDUCT_VEGETARIAN = 34, /* Obviously includes vegan */
	AID_CONDUCT_ATHEIST = 35,
	AID_CONDUCT_PACIFIST = 36,
	AID_CONDUCT_WEAPONLESS = 37,
	AID_CONDUCT_ILLITERATE = 38,
	AID_CONDUCT_NOPOLYMORPH = 39, /* Both polyless AND polyselfless! */
	AID_CONDUCT_GENOCIDELESS = 40,
	AID_CONDUCT_WISHLESS = 41,
	/* Achievements not awarded in-game */
	AID_CODE_COMMIT = 23,
	AID_BINGE_PARTICIPATE = 24,
	AID_BINGE_FULL_PARTICIPATE = 25
};

const static int ACHIEVEMENT_PUSH_SUCCESS = 1;
const static int ACHIEVEMENT_PUSH_FAILURE = 0;

bool achievement_system_startup();
void achievement_system_shutdown();

// Return ACHIEVEMENT_PUSH_* values above
int award_achievement(int achievement_id);
int add_achievement_progress(int achievement_id, int add_progress);
int push_achievement_progress(int achievement_id, int new_progress);

int get_achievement_progress(int achievement_id);
int get_achievement_max_progress(int achievement_id);

// Return 1 if achievement is awarded, 0 if not
int get_achievement_awarded(int achievement_id);

char * get_achievement_name(int achievement_id);

void reset_single_game_achievements();

void disable_achievements();

int achievements_user_exists();
int achievements_register_user();

int check_db_connection();
#endif
