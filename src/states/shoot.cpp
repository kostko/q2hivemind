/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/shoot.h"
#include "planner/local.h"
#include "planner/global.h"
#include "planner/directory.h"
#include "mapping/map.h"
#include "logger.h"
#include "context.h"
#include "dispatcher.h"

#include <limits>

namespace HiveMind {

ShootState::ShootState(Context* context)
  : State(context, "shoot", 1000, true)
{
  Object::init();
  context->getDispatcher()->signalOpponentSpotted.connect(boost::bind(&ShootState::makeEligible, this, _1));
}

ShootState::~ShootState()
{
}

void ShootState::initialize(const boost::any &metadata)
{
  m_complete = false;

  // Remember start time of shoot state request
  m_shootStart = Timing::getCurrentTimestamp();
}

void ShootState::goodbye()
{
}

void ShootState::checkEvent()
{
  // Check for possible enemies
  int enemyId = getClosestEnemy();

  if (enemyId != NO_ENEMY) {
    // Emit a signal
    OpponentSpottedEvent event(m_gameState->entities[enemyId].origin);
    getContext()->getDispatcher()->emit(&event);
  }
}

void ShootState::processFrame()
{
  Vector3f origin = m_gameState->player.origin;
  m_moveTarget = m_moveDestination = origin;
  m_moveFire = false;
  m_moveJump = false;

  m_targetId = getClosestEnemy();

  // Complete state if there's no enemy to shoot at
  if (m_targetId == NO_ENEMY) {
    // Only learn if we have been in this state long enough
    timestamp_t now = Timing::getCurrentTimestamp();
    m_shouldLearn = (m_shootStart - now > MIN_SHOOT_TIME);
    return;
  }

  // Otherwise set the entity as the target and shoot
  m_moveTarget = m_gameState->entities[m_targetId].origin;
  m_moveFire = true;
}

int ShootState::getClosestEnemy()
{
  Map *map = getContext()->getMap();
  Directory *dir = getLocalPlanner()->getContext()->getGlobalPlanner()->getDirectory();
   
  // Select a (the closest) target to shoot at
  Vector3f origin = m_gameState->player.origin;
  double minDist = MIN_DISTANCE;
  int enemyId = NO_ENEMY;

  for (int i = 1; i < m_gameState->maxPlayers; i++) {
    // Check for player entity
    if (i == m_gameState->playerEntityId)
      continue;
    
    // Check entity visibility
    if (!m_gameState->entities[i].isVisible())
      continue;
    
    // Check for friendly fire
    if (dir->isFriend(i))
      continue;
    
    // Check if the enemy is alive
    Vector3f enemyPos = m_gameState->entities[i].origin;
    bool isAlive = false;

    for (int j = 0; j <= 3; j++) {
      // Ray test for different offsets
      enemyPos[2] = m_gameState->entities[i].origin[2] + ENEMY_OFFSETS[j];
      isAlive = (map->rayTest(origin, enemyPos, Map::Solid) >= 1.0);
      if (isAlive) {
        break;
      }
    }

    if (isAlive) {
      double newDist = (origin - enemyPos).norm();
      if (newDist < minDist) {
          minDist = newDist;
          enemyId = i;
      }
    }
  }

  return enemyId;
}

void ShootState::makeEligible(OpponentSpottedEvent *event)
{
  getLocalPlanner()->addEligibleState(this);
}


}
