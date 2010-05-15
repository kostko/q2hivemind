/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/respawn.h"
#include "planner/local.h"
#include "logger.h"
#include "context.h"

namespace HiveMind {

RespawnState::RespawnState(Context *context)
  : State(context, "respawn")
{
  Object::init();
}

RespawnState::~RespawnState()
{
}

void RespawnState::checkInterruption()
{
  int health = m_gameState->player.health;

  if (health < 0) {
    getLocalPlanner()->requestTransition("respawn", 500);
  }
}

void RespawnState::initialize(const boost::any &metadata, bool restored)
{
  getLogger()->info("Now entering respawn state.");
}

void RespawnState::goodbye()
{
  getLogger()->info("Now leaving respawn state.");
}

void RespawnState::processFrame()
{
    m_complete = true; // TODO: how to delete this state so it doesnt get entered into anymore?
    
    //getLocalPlanner()->requestTransition("wander", 1);
}

void RespawnState::processPlanning()
{
  // TODO implement respawn planning (if needed)
}

}


