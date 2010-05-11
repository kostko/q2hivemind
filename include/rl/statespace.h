/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_RL_STATESPACE_H
#define HM_RL_STATESPACE_H

#include <vector>

#include "rl/enumvector.h"
#include "rl/action.h"
#include "rl/brainstate.h"

using namespace std;

namespace HiveMind {


class StateSpace {
public:
    /**
     * Constructor.
     */
    StateSpace(vector<int> &stateComponents, vector<int> &actionComponents);
    
    /**
     * Destructor.
     */
    ~StateSpace();
    
    /**
     * Q value for state and action.
     */
    double& at(BrainState *state, BrainAction *action);
    
    /**
     * Maximum possible Q value for state and some action.
     */
    double max(BrainState *state);
    
    /**
     * Number of actions.
     */
    int actions() const { return m_action->permutations(); }

    /**
     * Number of states.
     */
    int states() const { return m_state->permutations(); }
    
    /**
     * State components def.
     */
    void getStateComponents(std::vector<int> &components);

    /**
     * Action components def.
     */
    void getActionComponents(std::vector<int> &components);
    
    /**
     * Sum of all elements.
     */
    double sum();
  
private:
    BrainState  *m_state;           // These two vectors define the space
    BrainAction *m_action;
    vector<double> m_data;          // The actual vector of Q values
};

}

#endif

