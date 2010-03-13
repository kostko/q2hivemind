/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "context.h"

using namespace HiveMind;

/**
 * Debug entry point.
 */
int main()
{
  Context *context = new Context("uniq-id");
  context->connectTo("::1", 27910);
  delete context;
  return 0;
}


