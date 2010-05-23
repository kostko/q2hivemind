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
     * @param context Context context
     * @param name State name
     * @param eligibilityTime int Time before deleted from eligible states set
     */
    State(Context *context, const std::string &name, int eligibilityTime);
    
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
     * @param restored True if state was restored from stack
     */
    virtual void initialize(const boost::any &metadata, bool restored) = 0;
    
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
     *
     * Default implementation does nothing.
     */
    virtual void processPlanning() {};

    /**
     * This method should implement state specific event
     * checking, so the state can emit a signal when
     * needed. This method is called in main thread context.
     *
     * Default implementation does nothing.
     */
    virtual void checkEvent() {};
    
    /**
     * Has the state reached a final state?
     */
    inline bool isComplete() const { return m_complete; };

    /**
     * Should we learn from this transition?
     *
     * For example if we switch to shoot state for 0.5 seconds we
     * don't acknowledge it as a useful transition.
     */
    inline bool shouldLearn() const { return m_shouldLearn; }

    /**
     * Returns this state's priority.
     */
    inline int getPriority() const { return m_priority; }

    /**
     * Returns when did trigger event for this state occur.
     */
    inline timestamp_t getEventStart() { return m_eventStart; }

    /**
     * Returns eligibility time.
     */
    inline int getEligibilityTime() { return m_eligibilityTime; }

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
     * Sets the start of event.
     * 
     * @param eventStart timestamp_t
     */
    inline void setEventStart(timestamp_t eventStart) { m_eventStart = eventStart; }

    /**
     * Return true if this bot is alive and false if he is dead.
     */
    inline bool alive() { return (m_gameState->player.health > 0); }
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

    // Learn from this state transition
    bool m_shouldLearn;
private:
    // Unique state name
    std::string m_name;
    
    // Context and local planner
    Context *m_context;
    LocalPlanner *m_planner;
    
    // Priority
    int m_priority;

    // When did the trigger event for this state occur
    timestamp_t m_eventStart;

    // How long should we leave the state in m_eligibleStates (to be a candidate
    // for transition) before deleting it [in ms]
    int m_eligibilityTime;

};

}

#endif

