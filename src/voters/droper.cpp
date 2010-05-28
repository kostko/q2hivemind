/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "voters/droper.h"
#include "algebra.h"
#include "context.h"
#include "planner/local.h"
#include "planner/global.h"

namespace HiveMind {

DroperVoter::DroperVoter(Context *context)
  : m_context(context)
{
}

DroperVoter::~DroperVoter()
{
}


PollVote DroperVoter::vote(Bot *requestor, const std::string &category)
{
  double distance;
  Bot *me = m_context->getGlobalPlanner()->getDirectory()->getBotByName(m_context->getBotId());
  
  if (!m_context->getLocalPlanner()->canDropWeapon())
    distance = 0;
  else
    distance = 1.0/(requestor->getOrigin() - m_context->getLocalPlanner()->getGameState()->player.origin).norm();

  return PollVote(me, distance);
}

}


