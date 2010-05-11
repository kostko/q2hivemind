/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "rl/brainstate.h"

namespace HiveMind {

BrainState::BrainState()
{
  BrainState("");
}

BrainState::BrainState(const std::string &name)
  : m_name(name)
{
}

void BrainState::init(std::vector<int> &components)
{
  EnumVector::init(components);
}

}
