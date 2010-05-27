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

#include <boost/thread.hpp>

namespace HiveMind {

namespace MOLD {
  namespace Protocol {
    class Message;
  }
}

class Context;
class Directory;
class Bot;
class BotRespawnEvent;
class EntityUpdatedEvent;
class Poll;
class PollVoter;

/**
 * Global planner.
 */
class GlobalPlanner : public Object {
public:
    // Out of age bot collection interval
    enum { bot_collect_interval = 30000 };
    
    // Bot update emission interval
    enum { bot_update_interval = 500 };
    
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
    
    /**
     * Creates and starts a new poll within the team. After calling
     * this method, the global planner will own this Poll instance.
     * You will be notified of poll completion via an event.
     *
     * @param poll Poll descriptor
     */
    void createPoll(Poll *poll);
    
    /**
     * Registers a new voter module.
     *
     * @param category Vote category
     * @param voter Voter instance
     */
    void registerVoter(const std::string &category, PollVoter *voter);
protected:
    /**
     * Called when a message is received via MOLD.
     *
     * @param msg The received message
     */
    void moldMessageReceived(const MOLD::Protocol::Message &msg);
    
    /**
     * Bot respawned event handler and propagator.
     */
    void botRespawned(BotRespawnEvent *event);
    
    /**
     * This method gets called when an entity has been updated.
     */
    void entityUpdated(EntityUpdatedEvent *event);
    
    /**
     * Searches for a bot in the directory and if one is not
     * found it requests an announcement from it.
     *
     * @param msg Message received from the bot
     * @return A valid Bot directory entry or NULL if there is none
     */
    Bot *getBotOrRequestAnnounce(const MOLD::Protocol::Message &msg);
private:
    // Context
    Context *m_context;
    
    // Bot directory
    Directory *m_directory;
    timestamp_t m_lastCollection;
    
    // Updates
    timestamp_t m_lastBotUpdate;
    Vector3f m_lastBotOrigin;
    
    // Registered active polls and local voters
    boost::unordered_map<std::string, Poll*> m_polls;
    boost::unordered_map<std::string, PollVoter*> m_pollVoters;
    boost::mutex m_pollMutex;
    
    // Last poll test
    timestamp_t m_lastPollTest;
};

}

#endif

