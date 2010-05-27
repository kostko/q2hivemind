/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "context.h"
#include "simulation/interface.h"

#include <iostream>

using namespace HiveMind;

/**
 * Creates a Hivemind context.
 */
context_t create_hivemind_context()
{
  Context *context = new Context("hSimul", "/usr/share/games/quake2", "/home/kostko/development/sola/pipt/hivemind/data", "male/flak", "explore");
  context->simulationStart("maps/q2dm1.bsp");
  return static_cast<context_t>(context);
}

void hivemind_world_update(context_t hm_context)
{
  Context *context = static_cast<Context*>(hm_context);
  Vector3f origin = Vector3f(bot_origin());
  Vector3f orientation = Vector3f(bot_orientation());
  
  // Construct a gamestate from this information
  GameState state;
  state.playerEntityId = bot_entity_id();
  state.maxPlayers = 32;
  state.player.angles = orientation;
  state.player.origin = origin;
  state.player.serverOrigin = origin;
  state.player.velocity = Vector3f::Zero(); // XXX
  state.player.health = bot_health();
  state.player.ammo = 100; // XXX
  state.player.armor = 0; // XXX
  state.player.frags = bot_frags();
  
  // TODO entities
  
  // Simulate a frame
  Vector3f angles, velocity;
  context->simulateFrame(state, &angles, &velocity);
  
  // Request the bot to move
  bot_move(angles.data(), velocity.data());
}

