/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "rl/statespace.h"

StateSpace::StateSpace(vector<int> &stateComponents, vector<int> &actionComponents)
{
  m_state = new State(stateComponents);
  m_action = new Action(actionComponents);
  
  // Allocate the needed space for all Q values
  m_data = vector<double>(m_state->permutations() * m_action->permutations(), 0);
}
  
StateSpace::~StateSpace()
{
  delete m_state;
  delete m_action;
}
  
double& StateSpace::at(State *state, Action *action)
{
  int id = action->id() * state->permutations() + state->id();
  return m_data[id + 1];
}

double StateSpace::max(State *state)
{
  Action *action = m_action;
  int p = actions();         // All actions
  action->from(0);           // Start the search at action number 0

  // Current maximum           
  double maxQ = at(state, action);
  
  for (int i = 1; i < p; i++) {
    
    // "Compute" action i and check its Q value
    action->from(i);
    double tmpQ = at(state, action);
    
    if (tmpQ > maxQ) {
      maxQ = tmpQ;
    }
  }
  
  return maxQ;
}

double StateSpace::sum()
{
  double sum = 0;

  for (int i = 0; i < m_data.size(); i++)
    sum += m_data[i];
    
  return sum;
}
