/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/state.h"
#include "context.h"

#include <list>

namespace HiveMind {

State::State(Context *context, const std::string &name)
  : m_name(name),
    m_context(context),
    m_planner(context->getLocalPlanner()),
    m_priority(1)
{
}

State::~State()
{
}

void State::getNextTarget(Vector3f *destination, Vector3f *target, bool *fire) const
{
  *destination = m_moveDestination;
  *target = m_moveTarget;
  *fire = m_moveFire;
}

}


