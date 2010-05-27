/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_DISPATCHER_H
#define HM_DISPATCHER_H

#include "object.h"
#include "event.h"

#include <boost/signals2.hpp>
#include <boost/thread.hpp>

namespace HiveMind {

class Context;

/**
 * Represents the event dispatcher.
 */
class Dispatcher : public Object {
public:
    /**
     * Class constructor.
     *
     * @param context Bot context
     */
    Dispatcher(Context *context);
    
    /**
     * Emits an event (a convenience method so you don't need to emit
     * via different signals).
     *
     * @param event Event instance
     */
    void emit(Event *event);
    
    /**
     * Emits a deferred event. It will be delivered later in main
     * thread context. Note that in contrast to emit method, this
     * assumes that events are allocated ON THE HEAP and are considered
     * OWNED BY DISPATCHER! They will be automatically deleted after
     * delivery. Passing in a temporary will CRASH!
     *
     * @param event Event instance (heap allocated)
     */
    void emitDeferred(Event *event);
    
    /**
     * Delivers deferred events.
     */
    void deliver();
public:
    // Signals (please note that they are delivered in EMITTING THREAD CONTEXT)
    boost::signals2::signal<void (Event *event)> signalAnyEvent;
    boost::signals2::signal<void (BotLocationUpdateEvent *event)> signalBotLocationUpdate;
    boost::signals2::signal<void (BotRespawnEvent *event)> signalBotRespawn;
    boost::signals2::signal<void (OpponentSpottedEvent *event)> signalOpponentSpotted;
    boost::signals2::signal<void (EntityUpdatedEvent *event)> signalEntityUpdated;
    boost::signals2::signal<void (PollVoteCompletedEvent *event)> signalPollVoteCompleted;
private:
    // Context
    Context *m_context;
    
    // Deferred events
    std::list<Event*> m_eventQueue;
    boost::mutex m_eventQueueMutex;
};

}

#endif

