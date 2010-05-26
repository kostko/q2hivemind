/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */ 
#include "planner/local.h"
#include "rl/brains.h"
#include "logger.h"

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
  delete m_currState;
  delete m_currAction;
  delete m_tempAction;
  delete m_tempState;
}

void Brains::init(vector<int> &stateComponents, vector<int> &actionComponents)
{
  m_Q = new StateSpace(stateComponents, actionComponents);
  m_numQ = new StateSpace(stateComponents, actionComponents);

  m_suggestedAction = newBrainAction();
  
  m_currAction = newBrainAction("NULL");
  m_tempAction = newBrainAction();

  m_currState = newBrainState("NULL");
  m_tempState = newBrainState();
  
  // Initialize seed
  srand(time(0));
}

void Brains::interact()
{


  // If current state is not complete or if we don't have possible alternative states to transition to, then skip
  if (!m_localPlanner->getCurrentState()->isComplete() || !m_localPlanner->alternativeStates())
    return;  
  
  if (m_currAction->getName() == defaultActionName()) {
    // We are in wander state and there are some other eligible states available
    
    // Observe my state
    *m_currState = *observe();

    if (m_learn) {
      *m_currAction = *randomAction();
    } else {
      *m_currAction = *exploit(m_currState);
    }

    // Execute my chosen action
    execute(m_currAction);
  }

  // Just completed an action - learn.
  else {
    // Observe my new state
    BrainState *newState = observe();

    // Should we learn from this action?
    bool useful = m_currAction->executionState()->shouldLearn();

    // If I am learning try to learn from this move
    if (m_learn && useful) {
      getLogger()->info("Learning from state transition.");
      updateQ(m_currState, m_currAction, newState);
    }

    // Fallback to default action
    m_currAction->from(alwaysEligibleId());
    execute(m_currAction);
  }  
}

void Brains::setBrainMode(bool learn)
{
  m_learn = learn;
}

StateSpace *Brains::getQ()
{
  return m_Q;
}

BrainState *Brains::newBrainState(std::string name)
{
  vector<int> comps;
  getQ()->getStateComponents(comps);
  BrainState *bs = new BrainState(name);
  bs->init(comps);

  return bs;
}

BrainAction *Brains::newBrainAction(std::string name)
{
  vector<int> comps;
  getQ()->getStateComponents(comps);
  BrainAction *ba = new BrainAction(name);
  ba->init(comps);
  
  return ba;
}

BrainAction *Brains::randomAction()
{
  while (true) {
    int actionID = (int)(random() * m_Q->actions());
    m_suggestedAction->from(actionID);

    // The action must be possible to execute
    if (eligibleAction(m_suggestedAction)) {
      break;
    }
  }
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
  } else {
    double e = total / CEILING;
    return MIN_Q_TEMPERATURE + (1-e) * (MAX_Q_TEMPERATURE - MIN_Q_TEMPERATURE);
  }
}

BrainAction *Brains::exploit(BrainState *state)
{
  int best = alwaysEligibleId();
  m_suggestedAction->from(best);
  double maxVal = m_Q->at(state, m_suggestedAction);

  // Return the action with the highest Q value
  for (int i = 0; i < m_Q->actions(); i++) {

    m_suggestedAction->from(i);
    double newVal = m_Q->at(state, m_suggestedAction);

    if (newVal > maxVal && eligibleAction(m_suggestedAction)) {
      maxVal = newVal;
      best = i;
    }
  }

  m_suggestedAction->from(best);

  return m_suggestedAction;
}

}
