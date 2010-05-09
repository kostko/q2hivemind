/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/wander.h"
#include "logger.h"
#include "context.h"

namespace HiveMind {

WanderState::WanderState(Context *context)
  : State(context, "wander"),
    m_nextPoint(-1),
    m_speed(0)
{
  Object::init();
}

WanderState::~WanderState()
{
}

void WanderState::initialize(const boost::any &metadata)
{
  getLogger()->info("Now entering wander state.");
  
  // TODO Check metadata if it contains a destination
}

void WanderState::goodbye()
{
  getLogger()->info("Now leaving wander state.");
}

void WanderState::processFrame()
{
  Map *map = getContext()->getMap();
  
  // By default we stand still and do not fire
  m_moveTarget = m_moveDestination = m_gameState->player.origin;
  m_moveFire = false;
  
  // TODO this needs to be improved
  
  // Check if we got stuck
  if (m_speed > 0) {
    int delta = Timing::getCurrentTimestamp() - m_lastFrameUpdate;
    if (delta > 0)
      m_speed = 0.05*(1000 * (m_gameState->player.origin - m_lastGameState->player.origin).norm()/delta + 19.0*m_speed);
    
    m_lastFrameUpdate += delta;
  } else {
    m_lastFrameUpdate = Timing::getCurrentTimestamp();
    m_speed = 400.0;
  }
  
  // TODO Detect when we are falling (raycast?), then after we hit the ground force
  //      path recalculation
  
  // Follow current path
  if (m_nextPoint > -1) {
    for (int i = m_currentPath.length - 2; i >= m_nextPoint; i--) {
      float n = m_gameState->player.origin[2] - m_currentPath.points[i][2];
      if (n < -16 || n > 64)
        continue;
      
      Vector3f v = m_currentPath.points[i];
      v[2] = m_gameState->player.origin[2];
      if ((m_gameState->player.origin - v).norm() < 24.0) {
        // Consider this point visited when we come close enough
        m_nextPoint = i + 1;
        getLogger()->info(format("Got it. Next point is %d.") % m_nextPoint);
        
        Vector3f p = m_gameState->player.origin;
        getLogger()->info(format("My position is %f,%f,%f.") % p[0] % p[1] % p[2]);
        p = m_currentPath.points[m_nextPoint];
        getLogger()->info(format("Travelling to %f,%f,%f.") % p[0] % p[1] % p[2]);
        break;
      }
    }
    
    if (m_nextPoint == m_currentPath.length - 1) {
      // We have reached our destination
      getLogger()->info("Destination reached.");
      m_nextPoint = -2; // XXX
    } else {
      Vector3f p = m_currentPath.points[m_nextPoint];
      p = m_gameState->player.origin;
      m_moveTarget = m_moveDestination = m_currentPath.points[m_nextPoint];
    }
    
    if (m_speed < 10) {
      // Mark this link as bad and request to recompute the path
      map->markLinkInvalid(m_currentPath.links[(m_nextPoint - 1) / 2]);
      m_speed = -1;
      m_nextPoint = -1;
      
      getLogger()->info("We are stuck, but should be following a path!");
    }
  }
}

void WanderState::processPlanning()
{
  Vector3f p = m_gameState->player.origin;
  Map *map = getContext()->getMap();
  MapPath path;
  
  // Plan a path if none is currently available
  if (m_nextPoint == -1) {
    getLogger()->info(format("I am at position %f,%f,%f and have no next point.") % p[0] % p[1] % p[2]);
    //if (map->randomPath(p, &m_currentPath))
    if (map->findPath(p, Vector3f(1888.0,736.0,546.0), &m_currentPath, true)) {
      getLogger()->info(format("Discovered a path of length %d.") % m_currentPath.length);
      m_nextPoint = 0;
    } else {
      getLogger()->info("Path not found.");
      m_nextPoint = -2;
    }
  }
}

}


