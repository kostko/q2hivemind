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
#include "dispatcher.h"
#include "event.h"

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
  
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (client) {
    // Connect to message received signal
    client->signalMessageReceived.connect(boost::bind(&GlobalPlanner::moldMessageReceived, this, _1));
  }
}

GlobalPlanner::~GlobalPlanner()
{
}

void GlobalPlanner::start()
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (!client) {
    // The client might not be available when we are inside the simulation
    // so in this case we simply ignore this and return
    return;
  }
  
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
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (!client) {
    // The client might not be available when we are inside the simulation
    // so in this case we simply ignore this and return
    return;
  }
  
  // Collect out of date bots
  if (now - m_lastCollection > bot_collect_interval) {
    m_directory->collect();
    m_lastCollection = now;
  }
  
  // Emit our update (this must always be the server origin not an interpolated one
  // as interpolated origins might be inside walls or off ledges)
  if (now - m_lastBotUpdate > bot_update_interval) {
    Vector3f origin = state.player.serverOrigin;
    
    if (origin != m_lastBotOrigin) {
      // Construct the location update event
      Protocol::LocationUpdate upd;
      upd.set_x(origin[0]);
      upd.set_y(origin[1]);
      upd.set_z(origin[2]);
      client->deliver(Protocol::Message::EVENT_LOCATION_UPDATE, &upd);
      
      // Reset timer and last bot origin
      m_lastBotUpdate = now;
      m_lastBotOrigin = origin;
    }
  }
}

Bot *GlobalPlanner::getBotOrRequestAnnounce(const MOLD::Protocol::Message &msg)
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  Bot *bot = m_directory->getBotByName(msg.sourceid());
  if (!bot) {
    // Bot not yet available in our directory, request annoucement first
    getLogger()->warning(format("Got an event from an unknown bot (%s), requesting annoucement.") % msg.sourceid());
    
    // Construct an announcement and send it
    Protocol::Announcement ann;
    ann.set_name(m_context->getBotId());
    ann.set_entityid(m_context->getConnection()->getPlayerEntityId());
    ann.set_reply(false);
    client->deliver(Protocol::Message::CONTROL_ANNOUNCE, &ann, msg.sourceid());
  }
  
  return bot;
}

void GlobalPlanner::moldMessageReceived(const Protocol::Message &msg)
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (!client) {
    // If we are here that means that the client has been destroyed
    // between updates, this is a problem and should be logged
    getLogger()->warning("MOLD client not available, but message received!");
    return;
  }
  
  switch (msg.type()) {
    case Protocol::Message::CONTROL_ANNOUNCE: {
      // Received new bot announcement
      Protocol::Announcement ann = message_cast<Protocol::Announcement>(msg);
      Bot *bot = m_directory->getBotByName(ann.name());
      if (!bot) {
        getLogger()->info(format("A new bot (%s) with entity id %d has joined us.") % ann.name() % ann.entityid());
        
        // Insert new bot into directory
        m_directory->registerBot(ann.name(), ann.entityid());
      } else {
        // Bot is already known, simply update its timestamp,
        bot->updateTime();
      }
      
      // If this is not a reply, we should announce ourselves to the bot as well
      if (!ann.reply()) {
        Protocol::Announcement rply;
        rply.set_name(m_context->getBotId());
        rply.set_entityid(m_context->getConnection()->getPlayerEntityId());
        rply.set_reply(true);
        client->deliver(Protocol::Message::CONTROL_ANNOUNCE, &rply, ann.name());
      }
      break;
    }
    
    case Protocol::Message::EVENT_LOCATION_UPDATE: {
      // Location update event
      Protocol::LocationUpdate upd = message_cast<Protocol::LocationUpdate>(msg);
      Bot *bot = getBotOrRequestAnnounce(msg);
      if (bot) {
        // Update bot's origin and update time
        bot->setOrigin(Vector3f(upd.x(), upd.y(), upd.z()));
        bot->updateTime();
        
        // Generate an update event
        BotLocationUpdateEvent event(bot, bot->getOrigin());
        m_context->getDispatcher()->emit(&event);
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


