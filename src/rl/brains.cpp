/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */ 
#include "planner/local.h"
#include "rl/brains.h"

namespace HiveMind {

Brains::Brains(LocalPlanner *planner)
  : m_localPlanner(planner),
    m_learn(true)
{
  Object::init();
}

Brains::~Brains()
{
  delete m_Q;
  delete m_numQ;
  delete m_suggestedAction;
  delete m_tempAction;
}

void Brains::init(vector<int> &stateComponents, vector<int> &actionComponents)
{
  m_Q = new StateSpace(stateComponents, actionComponents);
  m_numQ = new StateSpace(stateComponents, actionComponents);

  m_suggestedAction = new BrainAction();
  m_suggestedAction->init(actionComponents);

  m_tempAction = new BrainAction();
  m_tempAction->init(actionComponents);
  
  // Initialize seed
  srand(time(0));
}

void Brains::interact(BrainState *currState, BrainAction *currAction)
{
  // Check in what state am I before the action
  *currState = *observe();
  
  if (m_learn) {
    *currAction = *randomAction();
  }
  else {
    *currAction = *exploit(currState);
  }
  
  // Execute my chosen action
  execute(currAction);
}

void Brains::learn(BrainState *beforeState, BrainAction *action)
{
  // Observe my new state
  BrainState *newState = observe();
  
  // If I am learning try to learn from this move
  if (m_learn)
    updateQ(beforeState, action, newState);
}

void Brains::setBrainMode(bool learn)
{
  m_learn = learn;
}

StateSpace *Brains::getQ()
{
  return m_Q;
}

BrainAction *Brains::randomAction()
{  
  int actionID = (int)(random() * m_Q->actions());
  m_suggestedAction->from(actionID);
  
  // TODO: a map ID -> State which executes the action
  
  return m_suggestedAction;
}

void Brains::updateQ(BrainState *prevState, BrainAction *action, BrainState *currState)
{
  double total = reward(prevState, currState) + (GAMMA * m_Q->max(currState));
  
  // Increment the visit counter
  m_numQ->at(prevState, action)++;

  // Learning rate
  double ALPHA = 1 / m_numQ->at(prevState, action);
  
  // Finally update Q value
  m_Q->at(prevState, action) = (1-ALPHA) * m_Q->at(prevState, action) + ALPHA * total;
}

double Brains::computeSigma(BrainState *state, double temperature)
{
  int p = m_Q->actions();
  double sigma = 0;
  
  for (int i = 0; i < p; i++) {
    m_tempAction->from(i);
    sigma += exp(m_Q->at(state, m_tempAction) / temperature);
  }
  
  return sigma;
}

BrainAction *Brains::suggestBoltz(BrainState *state, double temperature)
{
  double sigma = computeSigma(state, temperature);
  double prob = random();
  double sum = 0;
  int i = 0;
  
  while (sum < prob) {
    m_tempAction->from(i);
    sum += exp(m_Q->at(state, m_tempAction) / temperature) / sigma;
    i++;
  }
  
  *m_suggestedAction = *m_tempAction;
  
  return m_suggestedAction;
}

BrainAction *Brains::suggestAction(BrainState *state)
{
  return suggestBoltz(state, computeReasonableTemp());
}

double Brains::computeReasonableTemp()
{
  // Number of all "experiences"
  long int total = (long int) m_numQ->sum();

  if (total > CEILING) {
    return MIN_Q_TEMPERATURE;
  } 
  else {
    double e = total / CEILING;
    return MIN_Q_TEMPERATURE + (1-e) * (MAX_Q_TEMPERATURE - MIN_Q_TEMPERATURE);
  }
}

BrainAction *Brains::exploit(BrainState *state)
{
  return suggestBoltz(state, MIN_Q_TEMPERATURE);
}

}
