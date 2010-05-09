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

namespace HiveMind {

namespace MOLD {
  namespace Protocol {
    class Message;
  }
}

class Context;

/**
 * Global planner.
 */
class GlobalPlanner : public Object {
public:
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
};

}

#endif

