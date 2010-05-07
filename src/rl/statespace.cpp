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
  m_state = new EnumVector(stateComponents);
  m_action = new EnumVector(actionComponents);
  
  // Allocate the needed space for all Q values
  m_data = vector<double>(m_state->permutations() * m_action->permutations(), 0);
}
  
StateSpace::~StateSpace()
{
  delete m_state;
  delete m_action;
  delete &m_data;
}
  
double& StateSpace::at(EnumVector *state, EnumVector *action)
{
  int id = action->id() * state->permutations() + state->id();
  return m_data[id + 1];
}
