#ifndef HM_SIMULATION_LOCAL
#define HM_SIMULATION_LOCAL

// ------------------------------------------------------------------------------------------
// Game globals
// ------------------------------------------------------------------------------------------

#define MAX_BOTS		1
#define STEP_SIZE		18
#define PLAYER_HEIGHT	90

typedef struct botgame_s
{
  int human_players;
  int bot_players;
  edict_t *bots[MAX_BOTS];
  edict_t *game_controller;
  qboolean spawned;
  
  // Hivemind context
  context_t hm_context;
} botgame_t;

extern botgame_t *botgame;

void game_create();
void game_start();
void game_close();

void controller_think(edict_t *self);

// ------------------------------------------------------------------------------------------
// Bot data
// ------------------------------------------------------------------------------------------

typedef struct botinfo_s
{
  qboolean active;
  int effects;

  vec3_t prev_origin;
  vec3_t prev_angles;

  vec3_t desired_angles;
  int requested_turn;
  vec3_t desired_velocity;
  int requested_move;
  
  unsigned int body;
  usercmd_t command;
} botinfo_t;

void bot_respawn(edict_t *self);
void bot_think(edict_t *self);

void safe_centerprintf(edict_t* ent, char* fmt, ...);
void safe_cprintf(edict_t* ent, int printlevel, char* fmt, ...);
void safe_bprintf(int printlevel, char* fmt, ...);

#endif

