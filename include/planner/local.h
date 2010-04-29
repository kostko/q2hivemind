/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_PLANNER_LOCAL_H
#define HM_PLANNER_LOCAL_H

#include "object.h"
#include "planner/state.h"
#include "network/gamestate.h"

#include <list>
#include <boost/thread.hpp>
#include <boost/any.hpp>

namespace HiveMind {

/**
 * A transition request.
 */
class TransitionRequest {
public:
    // State to transition into
    std::string state;
    
    // Priority (higher number means higher priority)
    int priority;
    
    // Metadata (state-specific)
    boost::any metadata;
    
    /**
     * Comparison function.
     */
    friend inline bool operator==(const TransitionRequest &a, const TransitionRequest &b)
    {
      return a.state == b.state && a.priority == b.priority;
    }
};

/**
 * Local planner and state manager.
 */
class LocalPlanner : public Object {
public:
    /**
     * Class constructor.
     *
     * @param context Hivemind context
     */
    LocalPlanner(Context *context);
    
    /**
     * Class destructor.
     */
    virtual ~LocalPlanner();
    
    /**
     * Registers a new state for use by the planner.
     *
     * @param state State instance
     */
    void registerState(State *state);
    
    /**
     * Starts the background planning task.
     */
    void start();
    
    /**
     * Called on each frame after the world might have updated.
     *
     * @param state State of the world
     */
    void worldUpdated(const GameState &state);
    
    /**
     * Returns the next best move as it has been computed by
     * the planner. This move is actually computed in the background
     * and is just returned here.
     *
     * @param orientation Orientation vector pointer
     * @param velocity Velocity vector pointer
     * @param fire Fire flag pointer
     */
    void getBestMove(Vector3f *orientation, Vector3f *velocity, bool *fire) const;
    
    /**
     * Request a transition to the specified state. This doesn't
     * mean that this transition will actually get chosen as it may
     * get overriden by some higher-priority request.
     *
     * @param request A transition request
     */
    void requestTransition(const TransitionRequest &request);
    
    /**
     * Request a transition to the specified state. This doesn't
     * mean that this transition will actually get chosen as it may
     * get overriden by some higher-priority request.
     *
     * This method accepts state name and priority instead of a
     * transition request for conveniance purpuses. No metadata
     * can be passed using this method.
     *
     * @param state State name
     * @param priority Optional priority
     */
    void requestTransition(const std::string &state, int priority = 1);
protected:
    /**
     * Main processing loop for the local planner.
     */
    void process();
private:
    // Context
    Context *m_context;
    
    // State registry
    boost::unordered_map<std::string, State*> m_states;
    State *m_currentState;
    
    // Transition requests are aggregated and ranked, the highest ranking
    // request is approved if last state transition was not to close to
    // avoid state flapping
    std::list<TransitionRequest> m_transitionRequests;
    unsigned int m_lastTransition;
    
    // Background processing thread
    boost::thread m_workerThread;
    boost::mutex m_stateMutex;
    boost::mutex m_requestMutex;
    
    // Last game state
    GameState m_gameState;
    
    // Abort request flag
    bool m_abort;
};

}

#endif

