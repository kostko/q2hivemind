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
#include "logger.h"
#include "context.h"
#include "dispatcher.h"
#include <limits>

namespace HiveMind {

ShootState::ShootState(Context* context)
  : State(context, "shoot", 2000)
{
  Object::init();
  context->getDispatcher()->signalOpponentSpotted.connect(boost::bind(&ShootState::makeEligible, this, _1));
}

ShootState::~ShootState()
{
}

void ShootState::initialize(const boost::any &metadata, bool restored)
{
  getLogger()->info("Now entering shoot state.");
}

void ShootState::goodbye()
{
  getLogger()->info("Now leaving shoot state.");
}

void ShootState::checkEvent()
{
  // Check for possible enemies
  int enemyId = getClosestEnemy();

  if (enemyId != NO_ENEMY) {
    // Remember start time of shoot state request
    m_shootStart = Timing::getCurrentTimestamp();

    Vector3f origin = m_gameState->player.origin;

    // Emit a signal
    getContext()->getDispatcher()->emit(new OpponentSpottedEvent(origin));
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
    m_complete = true;
    //transitionDown();
    return;
  }

  // Otherwise set the entity as the target and shoot
  m_moveTarget = m_gameState->entities[m_targetId].origin;
  m_moveFire = true;
}

void ShootState::processPlanning()
{
  m_targetId = getClosestEnemy();
}

int ShootState::getClosestEnemy()
{
  // Select a (the closest) target to shoot at
  Vector3f origin = m_gameState->player.origin;
  double minDist = MIN_DISTANCE;
  int enemyId = NO_ENEMY;

  for (int i = 1; i < m_gameState->maxPlayers; i++) {
    bool isFriend = getLocalPlanner()->getContext()->getGlobalPlanner()->getDirectory()->isFriend(i);
    if (i != m_gameState->playerEntityId && m_gameState->entities[i].isVisible() && !isFriend) {

      Vector3f enemyPos = m_gameState->entities[i].origin;
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
  timestamp_t now = Timing::getCurrentTimestamp();
  setEventStart(now);

  getLocalPlanner()->addEligibleState(this);
}


}
