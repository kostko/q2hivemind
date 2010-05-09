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

// Protocol buffer messages
#include "src/mold/control.pb.h"

#include <sstream>
#include <ctime>

using namespace HiveMind::MOLD;

namespace HiveMind {

GlobalPlanner::GlobalPlanner(Context *context)
  : m_context(context)
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
  
  // We have a connection with MOLD message bus, send a ping message to everyone
  Protocol::Ping ping;
  ping.set_message("Hello world!");
  client->deliver(Protocol::Message::CONTROL_PING, &ping);
}

void GlobalPlanner::moldMessageReceived(const Protocol::Message &msg)
{
  getLogger()->info("Got message!");
  if (msg.type() == Protocol::Message::CONTROL_PING) {
    Protocol::Ping ping = message_cast<Protocol::Ping>(msg);
    getLogger()->info(format("Got CONTROL_PING: %s") % ping.message());
  }
}

}


