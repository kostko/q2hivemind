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
#include "dispatcher.h"

namespace HiveMind {

RespawnState::RespawnState(Context *context)
  : State(context, "respawn", 1000),
    m_selected(false)
{
  Object::init();
}

RespawnState::~RespawnState()
{
}

void RespawnState::initialize(const boost::any &metadata, bool restored)
{
  getLogger()->info(format("Now entering respawn state. Fire was %s") % m_moveFire);
  m_selected = true;
  m_complete = false;
  getLocalPlanner()->clearEligibleStates();
}

void RespawnState::goodbye()
{
  getLogger()->info("Now leaving respawn state.");
  m_selected = false;
  m_complete = true;
}

void RespawnState::processFrame()
{
  Vector3f dest = Vector3f(0.0, 0.0, 0.0);
  m_moveTarget = m_moveDestination = dest;
  m_moveFire = true;
  getLocalPlanner()->requestTransition("wander",1);
}

void RespawnState::checkEvent()
{
  int health = m_gameState->player.health;

  if (!m_selected && health < 0) {
    getLocalPlanner()->requestTransition("respawn", 500);
    
    // Emit respawn event
    BotRespawnEvent event(NULL);
    getContext()->getDispatcher()->emit(&event);
  }
}

}


