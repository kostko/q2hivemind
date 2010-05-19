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
public:
    // Signals (please note that they are delivered in EMITTING THREAD CONTEXT)
    boost::signals2::signal<void (Event *event)> signalAnyEvent;
    boost::signals2::signal<void (BotKilledEvent *event)> signalBotKilled;
    boost::signals2::signal<void (BotLocationUpdateEvent *event)> signalBotLocationUpdate;
    boost::signals2::signal<void (OpponentSpottedEvent *event)> signalOpponentSpotted;

private:
    // Context
    Context *m_context;
};

}

#endif

