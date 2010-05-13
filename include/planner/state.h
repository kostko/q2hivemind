/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_PLANNER_STATE_H
#define HM_PLANNER_STATE_H

#include "object.h"
#include "network/gamestate.h"

#include <list>
#include <boost/any.hpp>

namespace HiveMind {

class Context;
class LocalPlanner;

/**
 * A state the local planner (and therefore the bot) can be in
 * at any given time.
 */
class State : public Object {
friend class LocalPlanner;
public:
    /**
     * Class constructor.
     *
     * @param name State name
     */
    State(Context *context, const std::string &name);
    
    /**
     * Class destructor.
     */
    virtual ~State();
    
    /**
     * Returns this state's name.
     */
    inline std::string getName() const { return m_name; }
    
    /**
     * Prepare for entry into this state.
     *
     * @param metadata Supplied metadata
     */
    virtual void initialize(const boost::any &metadata) = 0;
    
    /**
     * Prepare for leaving this state.
     */
    virtual void goodbye() = 0;
    
    /**
     * This method should implement state specific processing on
     * each frame update. This method is called in main thread
     * context.
     */
    virtual void processFrame() = 0;
    
    /**
     * This method should implement state specific processing in
     * planning mode. This method is called in planner thread
     * context.
     */
    virtual void processPlanning() = 0;
    
    /**
     * This method should implement state specific interruption
     * requests, so the state can interrupt other states when
     * needed. This method is called in main thread context.
     *
     * Default implementation does nothing.
     */
    virtual void checkInterruption() {};
    
    /**
     * Has the state reached a final state?
     */
    inline bool isComplete() const { return m_complete; };
    
    /**
     * Returns this state's priority.
     */
    inline int getPriority() const { return m_priority; }
    
    /**
     * Returns the next best move as it has been computed by
     * the state.
     *
     * @param destination Destination vector pointer
     * @param target Target vector pointer
     * @param fire Fire flag pointer
     * @param jump Jump flag pointer
     */
    void getNextTarget(Vector3f *destination, Vector3f *target, bool *fire, bool *jump) const;
protected:
    /**
     * Returns the bot's context.
     */
    inline Context *getContext() const { return m_context; }
    
    /**
     * Returns the bot's local planner.
     */
    inline LocalPlanner *getLocalPlanner() const { return m_planner; }
    
    /**
     * Sets the state priority.
     *
     * @param priority Priority
     */
    inline void setPriority(int priority) { m_priority = priority; }
    
    /**
     * Requests the local planner to transition into a lower state down the
     * state stack.
     */
    void transitionDown();
protected:
    // Next move
    Vector3f m_moveDestination;
    Vector3f m_moveTarget;
    bool m_moveFire;
    bool m_moveJump;
    
    // Current and last game state
    GameState *m_gameState;
    GameState *m_lastGameState;
    
    // End of state
    bool m_complete;
private:
    // Unique state name
    std::string m_name;
    
    // Context and local planner
    Context *m_context;
    LocalPlanner *m_planner;
    
    // Priority
    int m_priority;
};

}

#endif

