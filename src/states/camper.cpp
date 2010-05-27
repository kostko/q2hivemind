/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/camper.h"
#include "planner/local.h"
#include "logger.h"
#include "context.h"
#include "dispatcher.h"
#include "rl/brains.h"

#include <limits>

namespace HiveMind {

CamperState::CamperState(Context *context)
  : State(context, "camper", -1, true)
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
}

void CamperState::goodbye()
{
}

void CamperState::processFrame()
{
  // Check if we have been in this state too long and exit state if so
  if (Timing::getCurrentTimestamp() - m_lastEntered > 3000) {
    getLocalPlanner()->requestTransition("wander");
  }
  
  // As we are camping, we stand still and are a sitting duck
  m_moveTarget = m_moveDestination = Vector3f(std::numeric_limits<float>::infinity(), 0, 0);
  m_moveFire = false;
  m_moveJump = false;
}

}


