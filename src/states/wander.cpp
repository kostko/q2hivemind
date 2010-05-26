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
#include "network/connection.h"
#include "mapping/grid.h"
#include "planner/local.h"

#include <limits>

namespace HiveMind {

WanderState::WanderState(Context *context)
  : State(context, "wander", -1, false),
    m_hasNextPoint(false),
    m_speed(0),
    m_minDistance(-1),
    m_randomize(false)
{
  Object::init();
}

WanderState::WanderState(Context *context, const std::string &name, int eligibilityTime, bool prunable)
  : State(context, name, eligibilityTime, prunable),
    m_hasNextPoint(false),
    m_speed(0),
    m_minDistance(-1),
    m_randomize(false)
{
  Object::init();
}

WanderState::~WanderState()
{
}

void WanderState::initialize(const boost::any &metadata)
{
  m_hasNextPoint = false;
  m_speed = 0;
  m_minDistance = -1;
  m_randomize = false;

  m_atDestination = false;

  // The wander state is always complete, because it has no goal.
  m_complete = true;
}

void WanderState::goodbye()
{
  recomputePath();
}

float WanderState::getDistanceToDestination() const
{
  return (m_currentPath.getCurrent()->getLocation() - m_gameState->player.origin).norm();
}

void WanderState::resetPointStatistics()
{
  m_minDistance = -1;
}

void WanderState::recomputePath(bool randomize)
{
  m_speed = -1;
  m_hasNextPoint = false;
  m_randomize = randomize;
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
      m_speed = 0.05*(1000 * (origin - m_lastOrigin).norm()/delta + 19.0*m_speed);

    m_lastFrameUpdate += delta;
  } else {
    m_lastFrameUpdate = now;
    m_speed = 400.0;
    m_lastOrigin = origin;
  }

  // Follow current path
  if (m_hasNextPoint) {
    // Visit current location and check if we got to a point
    if (m_currentPath.visit(origin)) {
      Vector3f p = m_currentPath.getCurrent()->getLocation();
      //getLogger()->info(format("Got it. Next point is %f %f %f.") % p[0] % p[1] % p[2]);
      resetPointStatistics();
    }

    if (m_currentPath.isDestinationReached()) {
      // We have reached our destination
      getLogger()->info("Destination reached.");
      m_atDestination = true;

      // We want to compute new random path the next time we will go into this state (in processPlanning)
      m_hasNextPoint = false;
      return;
    } else {
      m_moveTarget = m_moveDestination = m_currentPath.getCurrent()->getLocation();
    }

    if (m_speed < 10) {
      // Request to recompute the path
      //getLogger()->warning("We are stuck, but should be following a path!");
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
        //getLogger()->warning(format("Potential stalepoint detected at distance %f max diff %f Z diff %f!") % m_minDistance % diff % diffZ);

        if (diff > 25 && diffZ <= 24) {
          // Probably circling a waypoint
          //getLogger()->info("Probably circling a waypoint. Marking it as reached.");
          m_currentPath.skip();
          resetPointStatistics();
        } else if (diffZ > 24) {
          // Probably fell somewhere
          //getLogger()->info("Probably fell somewhere. Recomputing a new path.");
          recomputePath(true);
        } else {
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
  Grid *grid = getContext()->getGrid();

  // Plan a path if none is currently available
  if (!m_hasNextPoint) {
    //getLogger()->info(format("I am at position %f,%f,%f and have no next point.") % p[0] % p[1] % p[2]);
    if (grid->computeRandomPath(p, &m_currentPath, m_randomize)) {
      //getLogger()->info(format("Discovered a path of length %d.") % m_currentPath.size());
      m_hasNextPoint = true;
      resetPointStatistics();
    } else {
      //getLogger()->info("Path not found.");
    }
  }
}

}
