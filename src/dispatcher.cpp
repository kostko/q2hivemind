/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "dispatcher.h"
#include "logger.h"

#include <boost/signals2.hpp>
#include <boost/foreach.hpp>

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
  
  // Emit proper signals
  switch (event->getType()) {
    case Event::BotLocationUpdate: signalBotLocationUpdate(static_cast<BotLocationUpdateEvent*>(event)); break;
    case Event::BotRespawn: signalBotRespawn(static_cast<BotRespawnEvent*>(event)); break;
    case Event::OpponentSpotted: signalOpponentSpotted(static_cast<OpponentSpottedEvent*>(event)); break;
    case Event::EntityUpdated: signalEntityUpdated(static_cast<EntityUpdatedEvent*>(event)); break;
    default: break;
  }
  
  // Any handler is always called
  signalAnyEvent(event);
}

void Dispatcher::emitDeferred(Event *event)
{
  boost::lock_guard<boost::mutex> g(m_eventQueueMutex);
  m_eventQueue.push_back(event);
}
    
void Dispatcher::deliver()
{
  // Copy the event queue so we can deliver the events without holding the
  // event queue mutex
  std::list<Event*> queue;
  {
    boost::lock_guard<boost::mutex> g(m_eventQueueMutex);
    queue = m_eventQueue;
    m_eventQueue.clear();
  }
  
  // Mutex no longer held, we operate on its copy and own the events
  BOOST_FOREACH(Event *event, queue) {
    emit(event);
    delete event;
  }
}

}


