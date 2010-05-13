/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_PLANNER_GLOBAL_H
#define HM_PLANNER_GLOBAL_H

#include "object.h"
#include "timing.h"
#include "network/gamestate.h"

namespace HiveMind {

namespace MOLD {
  namespace Protocol {
    class Message;
  }
}

class Context;
class Directory;

/**
 * Global planner.
 */
class GlobalPlanner : public Object {
public:
    // Out of age bot collection interval
    enum { bot_collect_interval = 30000 };
    
    // Bot update emission interval
    enum { bot_update_interval = 250 };
    
    /**
     * Class constructor.
     *
     * @param context Hivemind context
     */
    GlobalPlanner(Context *context);
    
    /**
     * Class destructor.
     */
    virtual ~GlobalPlanner();
    
    /**
     * Start the global planner.
     */
    void start();
    
    /**
     * Called on each frame after the world might have updated.
     *
     * @param state State of the world
     */
    void worldUpdated(const GameState &state);
    
    /**
     * Returns the bot directory.
     */
    inline Directory *getDirectory() const { return m_directory; }
protected:
    /**
     * Called when a message is received via MOLD.
     *
     * @param msg The received message
     */
    void moldMessageReceived(const MOLD::Protocol::Message &msg);
private:
    // Context
    Context *m_context;
    
    // Bot directory
    Directory *m_directory;
    timestamp_t m_lastCollection;
    
    // Updates
    timestamp_t m_lastBotUpdate;
};

}

#endif

