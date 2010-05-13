/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/global.h"
#include "context.h"
#include "logger.h"
#include "mold/client.h"
#include "planner/directory.h"
#include "network/connection.h"

// Protocol buffer messages
#include "src/mold/control.pb.h"

#include <sstream>
#include <ctime>

using namespace HiveMind::MOLD;

namespace HiveMind {

GlobalPlanner::GlobalPlanner(Context *context)
  : m_context(context),
    m_directory(new Directory(context)),
    m_lastCollection(0),
    m_lastBotUpdate(0)
{
  Object::init();
  
  // Connect to message received signal
  m_context->getMOLDClient()->signalMessageReceived.connect(boost::bind(&GlobalPlanner::moldMessageReceived, this, _1));
}
    
GlobalPlanner::~GlobalPlanner()
{
}

void GlobalPlanner::start()
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (!client->isConnected()) {
    client->signalConnected.connect(boost::bind(&GlobalPlanner::start, this));
    return;
  }
  
  // We have a connection with MOLD message bus, send an annoucement to everyone
  Protocol::Announcement ann;
  ann.set_name(m_context->getBotId());
  ann.set_entityid(m_context->getConnection()->getPlayerEntityId());
  ann.set_reply(false);
  client->deliver(Protocol::Message::CONTROL_ANNOUNCE, &ann);
}

void GlobalPlanner::worldUpdated(const GameState &state)
{
  timestamp_t now = Timing::getCurrentTimestamp();
  
  // Collect out of date bots
  if (now - m_lastCollection > bot_collect_interval) {
    m_directory->collect();
    m_lastCollection = now;
  }
  
  // Emit our update
  if (now - m_lastBotUpdate > bot_update_interval) {
    // TODO
  }
}

void GlobalPlanner::moldMessageReceived(const Protocol::Message &msg)
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  
  switch (msg.type()) {
    case Protocol::Message::CONTROL_ANNOUNCE: {
      // Received new bot announcement
      Protocol::Announcement ann = message_cast<Protocol::Announcement>(msg);
      Bot *bot = m_directory->getBotByName(ann.name());
      if (!bot) {
        getLogger()->info(format("A new bot (%s) with entity id %d has joined us.") % ann.name() % ann.entityid());
        
        // Insert new bot into directory
        m_directory->registerBot(ann.name(), ann.entityid());
        
        // If this is not a reply, we should announce ourselves to the bot as well
        if (!ann.reply()) {
          Protocol::Announcement rply;
          rply.set_name(m_context->getBotId());
          rply.set_entityid(m_context->getConnection()->getPlayerEntityId());
          rply.set_reply(true);
          client->deliver(Protocol::Message::CONTROL_ANNOUNCE, &rply, ann.name());
        }
      } else {
        // Bot is already known, simply update its timestamp
        bot->updateTime();
      }
      break;
    }
    
    default: {
      // Message code not recongnized, emit a warning
      getLogger()->warning("Unrecognized MOLD message received from bus!");
      break;
    }
  }
}

}


