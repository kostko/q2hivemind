/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "context.h"
#include "network/connection.h"

namespace HiveMind {

Context::Context(const std::string &id)
  : m_botId(id),
    m_connection(NULL)
{
  Object::init();
}

Context::~Context()
{
}

void Context::connectTo(const std::string &host, unsigned int port)
{
  if (m_connection)
    return;
  
  m_connection = new Connection(m_botId, host, port);
  m_connection->connect();
  // TODO load maps here
  m_connection->begin();
  
  // XXX test
  for (int i = 0; i < 200; i++) {
    m_connection->move(
      Vector3f(0.0, (float) i / 50.0, 0.0),
      Vector3f(400.0, 0.0, 0.0),
      true
    );
    
    if (i == 100) {
      m_connection->say("Hello world!!");
    }
  }
}

}


