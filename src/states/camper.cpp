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
#include "mold/client.h"

#include <limits>

namespace HiveMind {

CamperState::CamperState(Context *context)
  : State(context, "camper", -1, true),
    m_alreadyStartedPoll(false),
    m_lastLocation(Vector3f(0,0,0))
{
  Object::init();
  getContext()->getDispatcher()->signalPollVoteCompleted.connect(boost::bind(&CamperState::voteCompleted, this, _1));
}

CamperState::~CamperState()
{
}

void CamperState::checkEvent()
{
}

void CamperState::voteCompleted(PollVoteCompletedEvent *event)
{
  if (event->getPoll()->getCategory() == "System.WhoWillDrop") {

    Bot *bot = event->getPoll()->getWinnerBot();
    if (bot) {
      // Send a confirmation message to the winner
      MOLD::ClientPtr client = getContext()->getMOLDClient();
      client->deliver(MOLD::Protocol::Message::DROP_CHOSEN, bot->getName());

      getLogger()->info(format("Sending a DROP_CHOSEN msg to %s.") % bot->getName());
    } else {
      // If there is no winner, don't wait in camper state
      getContext()->getLocalPlanner()->requestTransition("wander");
    }
  }
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

  // As we are camping, we stand still and are a sitting duck
  m_moveTarget = m_moveDestination = Vector3f(std::numeric_limits<float>::infinity(), 0, 0);

  m_moveFire = false;
  m_moveJump = false;
  m_lastLocation = m_gameState->player.origin;
}

}


