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
#include "planner/sensors.h"
#include "planner/motion.h"
#include "network/gamestate.h"
#include "rl/statespace.h"

#include <list>
#include <set>
#include <boost/thread.hpp>
#include <boost/any.hpp>

namespace HiveMind {

class Brains;
class BrainAction;

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
    void getBestMove(Vector3f *orientation, Vector3f *velocity, bool *fire);
        
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
     * @param metadata Optional metadata
     */
    void requestTransition(const std::string &state, const boost::any &metadata = boost::any());

    /**
     * Returns the game state object.
     */
    inline GameState *getGameState() { return m_gameState; }

    /**
     * Get context.
     */
    inline Context *getContext() { return m_context; }

    /**
     * Get brains.
     */
    inline Brains *getBrains() { return m_brains; }

    /**
     * Get state object from name.
     */
    inline State *getStateFromName(const std::string &name) { return m_states[name]; }

    /**
     * Add eligible state to m_eligibleStates.
     */
    void addEligibleState(State *state);

    /**
     * Remove an eligible state.
     */
    void removeEligibleState(State *state);

    /**
     * Prune too old states from m_eligibleStates.
     * Update eligible states set. Does two things:
     *   1) Calls checkEvent() method on all states. This will hopefully add 
     *      some new states to eligible states set if some trigger events occur.
     *   2) Prune too old states from eligible states set.
     */
    void updateEligibleStates();

    /**
     * Prune this state from eligible states set.
     *
     * @param state Name of the state
     */
    void pruneState(const std::string &state);

    /**
     * Is the given state eligible for transition?
     */
    inline bool isEligible(State *state) const { return m_eligibleStates.find(state) != m_eligibleStates.end(); }

    /**
     * Clear eligible states set.
     */
    void clearEligibleStates();

    /**
     * Are there any alternative states? Beside the always eligible state.
     */
    bool alternativeStates() const { return (isEligible(m_currentState) ? m_eligibleStates.size() > 1 : !m_eligibleStates.empty()); }

    /**
     * Returns the current state.
     */
    inline State *getCurrentState() { return m_currentState; }
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
        
    // Background processing thread
    boost::thread m_workerThread;
    boost::mutex m_requestMutex;
    
    // Current game state
    GameState *m_gameState;
    bool m_worldUpdated;
    
    // Abort request flag
    bool m_abort;
    
    // AI
    Brains *m_brains;
    
    // Motion controller
    MotionController m_motionController;

    // Eligible states to make transitions to.
    // This set should always be up-to-date for the current situation, so the Brains
    // will choose only from states that are eligible for the current situation.
    std::set<State*> m_eligibleStates;
};

}

#endif

