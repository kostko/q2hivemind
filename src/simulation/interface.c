/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "quake2/g_local.h"

// Global game instance
botgame_t *botgame = NULL;

// Bot entity
edict_t *cbot = NULL;

/**
 * Called by Quake2 on game initialization.
 */
void game_create()
{
  // Allocate the game context
  botgame = (botgame_t*) gi.TagMalloc(sizeof(botgame_t), TAG_GAME);
  memset(botgame, 0, sizeof(botgame_t));

  // Setup some variables
  gi.cvar("accel",   "0", CVAR_SERVERINFO);
  gi.cvar("bots",    "0", CVAR_SERVERINFO);
  gi.cvar("debug",   "0", CVAR_SERVERINFO);
  gi.cvar("obit",    "1", CVAR_SERVERINFO);
  gi.cvar("verbose", "0", CVAR_SERVERINFO);
  gi.cvar("freeze",  "0", CVAR_SERVERINFO);
}

/**
 * Called by Quake2 on game start.
 */
void game_start()
{
  edict_t *cl_ent;
  cvar_t *accel = gi.cvar("accel", "", 0);
  cvar_t *obit = gi.cvar("obit", "", 0);
  cvar_t *bots = gi.cvar("bots", "", 0);
  int i, clients = 0;

  // If we are already initialized, skip this
  if (botgame->game_controller)
    return;
  
  gi.dprintf("------- Hivemind Simulator Initialization -------\n");
  
  // Check for existing players
  for (i = 0; i < game.maxclients; i++) {
    cl_ent = g_edicts + 1 + i;
    if (cl_ent->inuse && cl_ent->client)
      clients++;
  }
  
  // Create a game entity to wrap the controller
  botgame->game_controller = G_Spawn();
  botgame->game_controller->classname = "controller";
  botgame->game_controller->think = controller_think;

  // Create hivemind context
  botgame->hm_context = create_hivemind_context();

  // Fix the next think to prevent weird happenings
  if (accel->value)
    botgame->game_controller->nextthink = level.time + 1000.0f;
  else
    botgame->game_controller->nextthink = level.time + 1.0f;
  
  if (accel->value) {
    gi.dprintf("Acceleration enabled!\n");
  } else {
    gi.dprintf("Acceleration disabled!\n");
  }
  
  gi.dprintf("Hivemind simulator initialized!\n");
}

/**
 * Called by Quake2 on game termination.
 */
void game_close()
{
}

/**
 * Called by Quake2 for global processing.
 */
void controller_think(edict_t *self)
{
  // Check if we need to spawn our bot and do so
  if (botgame->spawned)
    return;
  
  bot_connect("hm_hSimul");
  bot_config("male", "flak");
}

/**
 * Main processing function for the bot. This is where we dispatch
 * processing to Hivemind context and request changes.
 */
void bot_think(edict_t *self)
{
  int i;

  // Schedule next call
  self->nextthink = level.time + FRAMETIME;

  // Prepare command structure
  memset(&self->client->botinfo.command, 0, sizeof(usercmd_t));
  VectorSet(self->client->ps.pmove.delta_angles, 0, 0, 0);
  self->client->botinfo.command.msec = 100;

  // Respawn and initialise if dead
  if (self->deadflag == DEAD_DEAD) {
    self->client->botinfo.command.angles[PITCH] = 0;
    self->s.angles[PITCH] = 0;
    self->client->buttons = 0;				
    self->client->botinfo.command.buttons = BUTTON_ATTACK;
  } else {
    // Default is no movement
    for (i = 0; i < 3; i++) {
      self->client->botinfo.command.angles[i] = ANGLE2SHORT(self->client->ps.viewangles[i]);
    }
  }

  // Call hivemind to process the world
  hivemind_world_update(botgame->hm_context);
  self->client->botinfo.command.lightlevel = self->light_level;

  // Call code that handles clients to do the same processing
  ClientThink(self, &self->client->botinfo.command);

  // Fall over if we died
  if (self->deadflag == DEAD_DEAD) {
    self->client->botinfo.command.angles[PITCH] = 0;
    self->s.angles[PITCH] = 0;
  }

  // Reset the flags
  self->client->botinfo.requested_turn = false;
  self->client->botinfo.requested_move = false;
}

/**
 * Initializes the bot entity.
 */
void PutBotInServer(edict_t *ent)
{
  botinfo_t saved_bot_info;

  if (!deathmatch->value && !coop->value && !ctf->value) {
    gi.dprintf("Error - bot_create: Must be in deathmatch, coop or ctf!\n");
    return;
  }

  saved_bot_info = ent->client->botinfo;

  PutClientInServer(ent);

  ent->client->botinfo = saved_bot_info;
  ent->think = bot_think;
  ent->nextthink = level.time + FRAMETIME;
}

/**
 * Respawns the bot.
 */
void bot_respawn(edict_t *self)
{
  CopyToBodyQue(self);
  self->svflags &= ~SVF_NOCLIENT;
  PutBotInServer(self);

  self->s.event = EV_PLAYER_TELEPORT;
  self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
  self->client->ps.pmove.pm_time = 14;

  self->client->respawn_time = level.time;
}

/**
 * Creates a bot entity.
 */
void bot_connect(char* name)
{
  int i;
  char userinfo[MAX_INFO_STRING];
  edict_t *bot;
  cvar_t *bots = gi.cvar("bots", "", 0);
  char val[4];

  // Find a free edict
  for (i = maxclients->value; i > 0; i--) {
    bot = g_edicts + i + 1;
    if (!bot->inuse)
      break;
  }

  // Check if one was found
  if (bot->inuse) {
    safe_bprintf(PRINT_HIGH,"Error - bot_connect: The server is full!\n");
    return;
  }

  // Set skin properties
  memset(userinfo, 0, MAX_INFO_STRING);

  Info_SetValueForKey(userinfo, "name", name);
  Info_SetValueForKey(userinfo, "skin", "flak");
  Info_SetValueForKey(userinfo, "gender", "male");
  Info_SetValueForKey(userinfo, "hand", "2");

  // Set bot client information  
  bot->client->botinfo.active = true;
  bot->client->botinfo.body = 0;

  // Handle as a standard client
  ClientConnect(bot, userinfo);
  G_InitEdict(bot);
  InitClientResp(bot->client);
  PutBotInServer(bot);

  // Check if it went ok
  if (!bot->client) {
    safe_bprintf(PRINT_HIGH, "Error - bot_connect: The client is null.\n");
    return;
  }

  // Alert the server
  safe_bprintf(PRINT_HIGH, "%s entered the game.\n", bot->client->pers.netname);

  // Send muzzleflash over network
  gi.WriteByte(svc_muzzleflash);
  gi.WriteShort(bot - g_edicts);
  gi.WriteByte(MZ_LOGIN);
  gi.multicast(bot->s.origin, MULTICAST_PVS);

  // Done
  ClientEndServerFrame(bot);
  cbot = bot;

  ++botgame->bot_players;
  sprintf(val, "%i", (int) bots->value + 1);
  gi.cvar_set("bots", val);
}

/**
 * Configures a bot's model.
 */
void bot_config(char *appearance, char *gender)
{
  char userinfo[MAX_INFO_STRING];
  char tmp[24];

  if (!cbot)
    return;

  memcpy(userinfo, cbot->client->pers.userinfo, sizeof(userinfo));

  sprintf(tmp, "%s/%s", gender, appearance);
  Info_SetValueForKey(userinfo, "skin", tmp);
  Info_SetValueForKey(userinfo, "gender", gender);

  ClientUserinfoChanged(cbot, userinfo);
}

/**
 * Disconnects the bot.
 */
void bot_disconnect()
{
}

/**
 * Moves the bot.
 */
void bot_move(vec3_t orientation, vec3_t velocity)
{
  if (cbot->deadflag != DEAD_NO)
    return;

  // This is done relatively anyway, so just dump to command structure
  cbot->client->botinfo.command.forwardmove = velocity[0];
  cbot->client->botinfo.command.sidemove = velocity[1];
  cbot->client->botinfo.command.angles[YAW]	= ANGLE2SHORT(orientation[YAW] * 180.0f / M_PI);
	cbot->client->botinfo.command.angles[PITCH] = ANGLE2SHORT(orientation[PITCH] * 180.0f / M_PI);
  
  cbot->client->botinfo.requested_move = true;
  cbot->client->botinfo.requested_turn = true;
}

/**
 * Returns the bot origin.
 */
vec_t *bot_origin()
{
  return cbot->s.origin;
}

/**
 * Returns the bot orientation.
 */
vec_t *bot_orientation()
{
  return cbot->client->ps.viewangles;
}

/**
 * Returns the bot's entity identifier.
 */
unsigned int bot_entity_id()
{
  return cbot - g_edicts;
}

/**
 * Returns the bot's health.
 */
int bot_health()
{
  return cbot->health;
}

/**
 * Returns the bot's frags.
 */
int bot_frags()
{
  return cbot->client->ps.stats[STAT_FRAGS];
}


