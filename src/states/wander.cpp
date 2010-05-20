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
#include "mapping/grid.h"
#include "planner/local.h"

#include <limits>

namespace HiveMind {

WanderState::WanderState(Context *context)
  : State(context, "wander", -1),
    m_nextPoint(-1),
    m_speed(0),
    m_minDistance(-1),
    m_markInvalidOnNone(false),
    m_lastLink(NULL)
{
  Object::init();
  getLocalPlanner()->addEligibleState(this);
}

WanderState::~WanderState()
{
}

void WanderState::initialize(const boost::any &metadata, bool restored)
{
  getLogger()->info("Now entering wander state.");
  
  m_nextPoint = -1;
  m_speed = 0;
  m_minDistance = -1;
  m_markInvalidOnNone = false;
  m_lastLink = NULL;
  
  // TODO Check metadata if it contains a destination
}

void WanderState::goodbye()
{
  getLogger()->info("Now leaving wander state.");

  m_nextPoint = -1;
  m_speed = 0;
  m_minDistance = -1;
  m_markInvalidOnNone = false;
  m_lastLink = NULL;
}

Vector3f WanderState::getNextDestination() const
{
  return m_currentPath.points[m_nextPoint];
}

float WanderState::getDistanceToDestination() const
{
  return (m_currentPath.points[m_nextPoint] - m_gameState->player.origin).norm();
}

void WanderState::travelToPoint(int index)
{
  m_nextPoint = index;
  m_minDistance = -1;
  
  // Log some information about our destination
  Vector3f p = m_gameState->player.origin;
  getLogger()->info(format("My position is %f,%f,%f.") % p[0] % p[1] % p[2]);
  p = m_currentPath.points[m_nextPoint];
  getLogger()->info(format("Travelling to %f,%f,%f.") % p[0] % p[1] % p[2]);
}

MapLink *WanderState::getCurrentLink() const
{
  return m_currentPath.links[(m_nextPoint - 1) / 2];
}

void WanderState::recomputePath(bool markInvalidOnNone)
{
  //m_lastLink = getCurrentLink();
  m_markInvalidOnNone = markInvalidOnNone;
  m_speed = -1;
  m_nextPoint = -1;
}

void WanderState::processFrame()
{
  Map *map = getContext()->getMap();
  timestamp_t now = Timing::getCurrentTimestamp();
  Vector3f origin = m_gameState->player.origin;
  
  // By default we stand still and do not fire or jump
  m_moveTarget = m_moveDestination = Vector3f(std::numeric_limits<float>::infinity(), 0, 0);
  m_moveFire = false;
  m_moveJump = false;
  
  // Check if we got stuck
  if (m_speed > 0) {
    int delta = now - m_lastFrameUpdate;
    if (delta > 0)
      m_speed = 0.05*(1000 * (origin - m_lastGameState->player.origin).norm()/delta + 19.0*m_speed);
    
    m_lastFrameUpdate += delta;
  } else {
    m_lastFrameUpdate = now;
    m_speed = 400.0;
  }
  
  // Follow current path
  if (m_nextPoint > -1) {
    for (int i = m_currentPath.length - 2; i >= m_nextPoint; i--) {
      float n = m_gameState->player.origin[2] - m_currentPath.points[i][2];
      if (n < -16 || n > 64)
        continue;
      
      Vector3f v = m_currentPath.points[i];
      v[2] = origin[2];
      if ((origin - v).norm() < 24.0) {
        // Consider this point visited when we come close enough
        getLogger()->info(format("Got it. Next point is %d.") % (i + 1));
        travelToPoint(i + 1);
        break;
      }
    }
    
    if (m_nextPoint == m_currentPath.length - 1) {
      // We have reached our destination
      getLogger()->info("Destination reached.");
      m_nextPoint = -2; // XXX
      //m_complete = true;
      return;
    } else {
      m_moveTarget = m_moveDestination = getNextDestination();
    }

    if (m_speed < 10) {
      // Mark this link as bad and request to recompute the path
      getLogger()->warning("We are stuck, but should be following a path!");
      
      // TODO maybe this should just increase cost, not remove it completely
      //getCurrentLink()->invalidate();
      recomputePath();
    } else {
      // Check whether we will probably never reach our destination 
      float distance = getDistanceToDestination();
      
      if (m_minDistance == -1 || distance < m_minDistance) {
        m_minDistance = distance;
        m_maxDistance = distance;
        m_lastMinChange = now;
        m_lastZ = origin[2];
      } else if (now - m_lastMinChange > 1000) {
        float diff = m_maxDistance - m_minDistance;
        float diffZ = m_lastZ - origin[2];
        //
        // Reasons for stalepoint:
        //
        //   * Fell somewhere and our path is actually invalid (check via Z difference)
        //     (we must recalculate the path)
        //   * We are circling a waypoint
        //     (we must treat the waypoint as visited)
        //   * We are stuck because a link is unwalkable
        //     (we must mark the link as invalid and recalculate the path)
        //
        getLogger()->warning(format("Potential stalepoint detected at distance %f max diff %f Z diff %f!") % m_minDistance % diff % diffZ);
        
        if (diff > 25 && diffZ <= 24) {
          // Probably circling a waypoint
          getLogger()->info("Probably circling a waypoint. Marking it as reached.");
          travelToPoint(m_nextPoint + 1);
        } else if (diffZ > 24) {
          // Probably fell somewhere
          getLogger()->info("Probably fell somewhere. Recomputing a new path.");
          recomputePath(true);
        }
        
        return;
      }
      
      // Update max distance from last potential stalepoint when passed
      if (distance > m_maxDistance) {
        m_maxDistance = distance;
      }
    }
  }
}

void WanderState::processPlanning()
{
  Vector3f p = m_gameState->player.origin;
  Map *map = getContext()->getMap();
  Grid *grid = getContext()->getGrid();
  MapPath path;
  
  // Plan a path if none is currently available
  if (m_nextPoint == -1) {
    getLogger()->info(format("I am at position %f,%f,%f and have no next point.") % p[0] % p[1] % p[2]);
    if (grid->findPath(p, Vector3f(1888.0,736.0,546.0), &m_currentPath, true)) {
      getLogger()->info(format("Discovered a path of length %d.") % m_currentPath.length);
      travelToPoint(0);
    } else {
      getLogger()->info("Path not found.");
      m_nextPoint = -2;
    }
  }
}

}


