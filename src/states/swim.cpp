/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/swim.h"
#include "planner/local.h"
#include "logger.h"
#include "context.h"
#include "dispatcher.h"
#include "rl/brains.h"
#include <boost/signals2.hpp>

namespace HiveMind {

SwimState::SwimState(Context *context)
  : State(context, "swim", -1, true),
    m_firstInWater(0),
    m_lastInWater(0)
{
  Object::init();
}

SwimState::~SwimState()
{
}

void SwimState::checkEvent()
{
  Map *map = getContext()->getMap();
  Vector3f origin = m_gameState->player.origin;

  // Detect when we hit water or lava via raycasting
  if (map->rayTest(origin, origin + Vector3f(0, 0, 24.0), Map::Lava | Map::Water) < 0.5) {
    timestamp_t now = Timing::getCurrentTimestamp();
    if (!m_firstInWater)
      m_firstInWater = now;

    if (now - m_firstInWater > 1000) {
      // We have been in the water at least 1 second
      getLogger()->warning("We are sinking, I repeat we are sinking!");

      getLocalPlanner()->requestTransition("swim");
      m_firstInWater = 0;
    }
  } else {
    // Reset swim timer
    m_firstInWater = 0;
  }
}

void SwimState::initialize(const boost::any &metadata)
{
  m_complete = false;
}

void SwimState::goodbye()
{
}

void SwimState::processFrame()
{
  Map *map = getContext()->getMap();
  Vector3f origin = m_gameState->player.origin;

  m_moveJump = true;
  m_moveFire = false;

  // TODO: change this so the location is not hardcoded
  Vector3f dest = Vector3f(320.000000, 456.000000, 552.000000);
  m_moveTarget = m_moveDestination = dest;
    
  // Detect when we get out of water to transition to some other state down the stack
  if (map->rayTest(origin, origin + Vector3f(0, 0, 24.0), Map::Lava | Map::Water) > 0.5) {
    timestamp_t now = Timing::getCurrentTimestamp();
    if (!m_lastInWater)
      m_lastInWater = now;
    
    if (now - m_lastInWater > 1000) {
      // We have been out of the water at least 1 second
      
      m_complete = true;
      m_lastInWater = 0;
    }
  } else {
    // Reset swim timer
    m_lastInWater = 0;
  }

}

}


