/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/wander.h"
#include "logger.h"

namespace HiveMind {

WanderState::WanderState(Context *context)
  : State(context, "wander")
{
  Object::init();
}

WanderState::~WanderState()
{
}

void WanderState::initialize(const boost::any &metadata)
{
  getLogger()->info("Now entering wander state.");
}

void WanderState::goodbye()
{
  getLogger()->info("Now leaving wander state.");
}

void WanderState::processFrame(const GameState &state)
{
}

void WanderState::processPlanning()
{
}

}


