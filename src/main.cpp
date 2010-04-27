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
  context->connectTo("192.168.123.178", 27910);
  
  // XXX test
  Connection *connection = context->getConnection();
  for (int i = 0; i < 200; i++) {
    connection->move(
      Vector3f(0.0, (float) i / 50.0, 0.0),
      Vector3f(400.0, 0.0, 0.0),
      true
    );
    
    if (i == 100) {
      connection->say("Hello world!!");
    }
  }
  
  for (;;) { sleep(1); }
  delete context;
  return 0;
}


