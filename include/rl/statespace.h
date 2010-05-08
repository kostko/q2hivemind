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

using namespace std;

typedef EnumVector State;
typedef EnumVector Action;

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
  double& at(State *state, Action *action);
  
  /**
   * Maximum possible Q value for state and some action.
   */
  double max(State *state);
  
  /**
   * Number of actions.
   */
  int actions() const { return m_action->permutations(); }

  /**
   * Number of states.
   */
  int states() const { return m_state->permutations(); }
  
  /**
   * Sum of all elements.
   */
  double sum();
  
private:
  State  *m_state;           // These two vectors define the space
  Action *m_action;
  vector<double> m_data;     // The actual vector of Q values
};

#endif

