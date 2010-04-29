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
     * Returns this state's priority.
     */
    inline int getPriority() const { return m_priority; }
    
    /**
     * Returns the next best move as it has been computed by
     * the state.
     *
     * @param orientation Orientation vector pointer
     * @param velocity Velocity vector pointer
     * @param fire Fire flag pointer
     */
    void getNextMove(Vector3f *orientation, Vector3f *velocity, bool *fire) const;
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
protected:
    // Next move
    Vector3f m_moveOrientation;
    Vector3f m_moveVelocity;
    bool m_moveFire;
    
    // Last game state
    GameState *m_gameState;
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

