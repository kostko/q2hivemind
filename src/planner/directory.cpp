/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/directory.h"
#include "context.h"
#include "logger.h"

namespace HiveMind {

Bot::Bot(const std::string &id, int entityId)
  : m_name(id),
    m_entityId(entityId)
{
}

Directory::Directory(Context *context)
  : m_context(context)
{
  Object::init();
}

Bot *Directory::registerBot(const std::string &id, int entityId)
{
  Bot *bot = new Bot(id, entityId);
  m_names[id] = bot;
  m_entities[entityId] = bot;
  return bot;
}

Bot *Directory::unregisterBot(const std::string &id)
{
  Bot *bot = getBotByName(id);
  if (!bot)
    return NULL;
  
  m_names.erase(id);
  m_entities.erase(bot->getEntityId());
  return bot;
}

void Directory::updateEntityId(const std::string &id, int entityId)
{
  Bot *bot = getBotByName(id);
  if (!bot) {
    getLogger()->warning(format("Attempted to change entity id for non-existent bot (%s)!") % id);
    return;
  }
  
  m_entities.erase(bot->getEntityId());
  bot->setEntityId(entityId);
  m_entities[entityId] = bot; 
}

Bot *Directory::getBotByName(const std::string &id)
{
  if (m_names.find(id) != m_names.end())
    return m_names[id];
  
  return NULL;
}

Bot *Directory::getBotByEntityId(int entityId)
{
  if (m_entities.find(entityId) != m_entities.end())
    return m_entities[entityId];
  
  return NULL;
}

}


