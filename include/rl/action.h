/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_RL_ACTION_H
#define HM_RL_ACTION_H

#include "planner/state.h"
#include "rl/enumvector.h"
#include <string>

namespace HiveMind {

/**
 * Defines an action that a bot can make.
 *
 * The action is defined as a "physical" state the bot is in.
 */
class BrainAction : public EnumVector {
public:
    /**
     * Constructors.
     */ 
    BrainAction();
    BrainAction(const std::string &name);
    BrainAction(State *executionState, const std::string &name);
    
    /**
     * Destructor.
     */
    ~BrainAction();
    
    /**
     * Init.
     * @param components components[i] is the number of states that the i-th component can take.
     *                   Note: this also means that the values of component i go from 0 to components[i]-1!
     */
    void init(std::vector<int> &components);
    
    /**
     * Returns the state that executes the action.
     */
    inline State *executionState() { return m_executionState; }

    /**
     *  Sets the execution state.
     */
    inline void setExecutionState(State *execState) { m_executionState = execState; }
    
    /**
     * Is the action complete?
     */
    bool complete();
    
    /**
     * Action name.
     */
    inline std::string getName() const { return m_name; }

    /**
     * Set action name.
     */
    inline void setName(const std::string &name) { m_name = name; }
private:
  State *m_executionState;    // The state in which the bot executes this action
  std::string m_name;
};

}

#endif
