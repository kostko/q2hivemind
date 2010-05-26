/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/state.h"
#include "planner/local.h"
#include "context.h"

#include <list>

namespace HiveMind {

State::State(Context *context, const std::string &name, int eligibilityTime, bool prunable)
  : m_name(name),
    m_context(context),
    m_planner(context->getLocalPlanner()),
    m_priority(1),
    m_moveDestination(Vector3f::Zero()),
    m_moveTarget(Vector3f::Zero()),
    m_complete(false),
    m_eligibilityTime(eligibilityTime),
    m_eventStart(0),
    m_isPrunable(prunable)
{
}

State::~State()
{
}

void State::getNextTarget(Vector3f *destination, Vector3f *target, bool *fire, bool *jump) const
{
  *destination = m_moveDestination;
  *target = m_moveTarget;
  *fire = m_moveFire;
  *jump = m_moveJump;
}

}


