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

#include <boost/foreach.hpp>

namespace HiveMind {

Bot::Bot(const std::string &id, int entityId)
  : m_name(id),
    m_entityId(entityId)
{
  updateTime();
}

Directory::Directory(Context *context)
  : m_context(context)
{
  Object::init();
}

Bot *Directory::registerBot(const std::string &id, int entityId)
{
  Bot *bot = getBotByName(id);
  if (bot == NULL) {
    bot = new Bot(id, entityId);
    m_names[id] = bot;
    m_entities[entityId] = bot;
  } else {
    updateEntityId(id, entityId);
  }
  
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

Bot *Directory::getBotByName(const std::string &id) const
{
  if (m_names.find(id) != m_names.end())
    return m_names.at(id);
  
  return NULL;
}

Bot *Directory::getBotByEntityId(int entityId) const
{
  if (m_entities.find(entityId) != m_entities.end())
    return m_entities.at(entityId);
  
  return NULL;
}

void Directory::collect()
{
  std::list<Bot*> remove;
  typedef std::pair<std::string, Bot*> BotPair;
  
  BOOST_FOREACH(BotPair p, m_names) {
    Bot *bot = p.second;
    if (bot->getAge() > 60000) {
      // Collect bots older than 60 seconds
      remove.push_back(bot);
    }
  }
  
  // Remove all collected bots
  BOOST_FOREACH(Bot *bot, remove) {
    getLogger()->info(format("Removing bot %s from directory due to old age.") % bot->getName()); 
    unregisterBot(bot->getName());
  }
}

}


