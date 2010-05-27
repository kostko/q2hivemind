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
  : State(context, "respawn", -1, true),
    m_killedTime(0),
    m_i(0)
{
  Object::init();
}

RespawnState::~RespawnState()
{
}

void RespawnState::initialize(const boost::any &metadata)
{
  m_complete = false;
  getLocalPlanner()->clearEligibleStates();
}

void RespawnState::goodbye()
{
}

void RespawnState::processFrame()
{
  Vector3f origin = m_gameState->player.origin;
  m_moveTarget = m_moveDestination = origin;
  if (m_i % 2 == 0)
    m_moveFire = true;
  else
    m_moveFire = false;

  m_i++;

  if (m_i % 10 == 0)
    getLocalPlanner()->requestTransition("camper");
}

void RespawnState::checkEvent()
{
  int health = m_gameState->player.health;

  if (Timing::getCurrentTimestamp() - m_killedTime > 1000 && health < 0) {
    getLocalPlanner()->requestTransition("respawn");
    m_killedTime = Timing::getCurrentTimestamp();
    
    // Emit respawn event
    BotRespawnEvent event(NULL);
    getContext()->getDispatcher()->emit(&event);
  }
}

}


