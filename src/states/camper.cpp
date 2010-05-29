/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/camper.h"
#include "planner/local.h"
#include "planner/global.h"
#include "planner/poll.h"
#include "logger.h"
#include "context.h"
#include "dispatcher.h"
#include "rl/brains.h"
#include "mapping/grid.h"

#include <limits>

namespace HiveMind {

CamperState::CamperState(Context *context)
  : State(context, "camper", -1, true),
    m_alreadyStartedPoll(false),
    m_lastLocation(Vector3f(0,0,0))
{
  Object::init();
  
  // TODO We should subscribe to some drop event and reset m_lastEntered when received
}

CamperState::~CamperState()
{
}

void CamperState::checkEvent()
{
}

void CamperState::initialize(const boost::any &metadata)
{
  m_complete = false;
  m_lastEntered = Timing::getCurrentTimestamp();
  m_alreadyStartedPoll = false;
  m_lastLocation = m_gameState->player.origin;
}

void CamperState::goodbye()
{
}

void CamperState::processFrame()
{
  GridNode *node = getContext()->getGrid()->getNodeByLocation(m_gameState->player.origin);
  if (node->getType() == GridNode::SpawnPoint && !m_alreadyStartedPoll && (100.0 < (m_lastLocation - m_gameState->player.origin).norm())) {

    // Request a WhoWillDrop poll
    getContext()->getGlobalPlanner()->createPoll(new Poll(getContext(), Poll::VoteBot, 2000, "System.WhoWillDrop"));
    getLogger()->info(format("I HAVE JUST SPAWNED at %f %f %f") % m_gameState->player.origin.x() % m_gameState->player.origin.y() % m_gameState->player.origin.z());
    m_alreadyStartedPoll = true;
  }

  // Check if we have been in this state too long and exit state if so
  //if (Timing::getCurrentTimestamp() - m_lastEntered > 40000) {
  //  getLocalPlanner()->requestTransition("wander");
  //}

  m_moveTarget = m_moveDestination = Vector3f(std::numeric_limits<float>::infinity(), 0, 0);
    // As we are camping, we stand still and are a sitting duck

  m_moveFire = false;
  m_moveJump = false;
  m_lastLocation = m_gameState->player.origin;
}

}


