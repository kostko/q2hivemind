/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mapping/dynamic.h"
#include "mapping/grid.h"
#include "context.h"
#include "logger.h"

namespace HiveMind {

DynamicMapper::DynamicMapper(Context *context)
  : m_context(context),
    m_grid(context->getGrid())
{
  Object::init();
}

DynamicMapper::~DynamicMapper()
{
}

void DynamicMapper::worldUpdated(const GameState &state)
{
  // TODO when we see a new entity we learn its location into the grid
  //      and also dispatch events
  
  // TODO track our movements and learn paths
  
  // TODO incorporate team member movements to learn paths
  
  // TODO incorporate other entity movements to learn paths
}

}


