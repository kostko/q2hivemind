/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "dispatcher.h"
#include "logger.h"

namespace HiveMind {

Dispatcher::Dispatcher(Context *context)
  : m_context(context)
{
  Object::init();
  
  // Log our initialization
  getLogger()->info("Event dispatcher initialized.");
}

void Dispatcher::emit(Event *event)
{
  if (!event || !event->isValid()) {
    getLogger()->warning("Attempted to emit an invalid event!");
    return;
  }
  
  // Log this event
  getLogger()->info(format("Emitting event %s.") % event->getTypeAsString());
  
  // Emit proper signals
  switch (event->getType()) {
    case Event::BotKilled: signalBotKilled(static_cast<BotKilledEvent*>(event)); break;
    case Event::BotLocationUpdate: signalBotLocationUpdate(static_cast<BotLocationUpdateEvent*>(event)); break;
    default: break;
  }
  
  // Any handler is always called
  signalAnyEvent(event);
}

}


