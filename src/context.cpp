/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "context.h"
#include "logger.h"
#include "network/connection.h"
#include "mapping/map.h"

#include <boost/format.hpp>

using boost::format;

namespace HiveMind {

Context::Context(const std::string &id, const std::string &gamedir)
  : m_botId(id),
    m_gamedir(gamedir),
    m_connection(NULL),
    m_map(NULL)
{
  Object::init();
}

Context::~Context()
{
  delete m_connection;
  delete m_map;
}

void Context::connectTo(const std::string &host, unsigned int port)
{
  if (m_connection)
    return;
  
  m_connection = new Connection(m_botId, host, port);
  m_connection->connect();
  
  // Load maps
  getLogger()->info(format("Currently playing map: %s") % m_connection->getMapName());
  m_map = new Map(this, m_connection->getMapName());
  if (!m_map->open())
    getLogger()->error("Map open has failed, aborting now.");
  
  // Enter the game
  m_connection->begin();
}

}


