/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "rl/action.h"
#include "planner/state.h"

namespace HiveMind {

BrainAction::BrainAction()
{
  BrainAction("");
}

BrainAction::BrainAction(const std::string &name)
{
  BrainAction(NULL, name);
}

BrainAction::BrainAction(State *executionState, const std::string &name)
  : m_executionState(executionState),
    m_name(name)
{
}

BrainAction::~BrainAction()
{
}

void BrainAction::init(std::vector<int> &components)
{
  EnumVector::init(components);
}

bool BrainAction::complete()
{ 
  if (m_executionState != NULL) {
    return m_executionState->isComplete();
  } else {
    return true;
  }
}

}
