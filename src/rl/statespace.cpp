/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_RL_STATESPACE_H
#define HM_RL_STATESPACE_H

#include "rl/enumvector.h"
#include <vector>

using namespace std;

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
  
  double& at(EnumVector *state, EnumVector *action);
  
private:
  EnumVector *m_state;       // These two vectors define the space
  EnumVector *m_action;
  vector<double> m_data;     // The actual vector of Q values
};

#endif
