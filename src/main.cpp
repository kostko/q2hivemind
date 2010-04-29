/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "context.h"
#include "network/connection.h"

using namespace HiveMind;

/**
 * Debug entry point.
 */
int main()
{
  Context *context = new Context("uniq-id", "/usr/share/games/quake2");
  context->connectTo("::1", 27910);
  context->execute();
  delete context;
  return 0;
}


