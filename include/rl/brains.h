/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_RL_BRAINS_H
#define HM_RL_BRAINS_H

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "rl/enumvector.h"
#include "rl/statespace.h"

/**
 * Some constants of the Boltzmann "soft-max" control policy.
 */
#define GAMMA              0.6
#define MAX_Q_TEMPERATURE  1.0 / 2
#define MIN_Q_TEMPERATURE  1.0 / 50
#define CEILING            100000

/**
 * Random -> [0,1).
 */
#define random() ((double) rand()) / RAND_MAX

/**
 * Represents the "brains" of an agent.
 *
 * Concept taken from: http://www.compapp.dcu.ie/~humphrys/Notes/RL/Code/index.html
 */ 
class Brains {
public:
  /**
   * Constructor.
   */
  Brains(vector<int> &stateComponents, vector<int> &actionComponents);

  /**
   * Destructor.
   */
  ~Brains();

  /**
   * What really defines the "brain" is the reward function.
   */
  virtual double reward(State *prevState, State *currState) { return 0; }
  
    /**
   * Update Q function.
   */ 
  void updateQ(State *prevState, Action *action, State *currState);
  
  /**
   * Suggests a random action.
   */
  Action *randomAction();
  
  /**
   * Suggest an action using the soft-max policy.
   */
  Action *suggestBoltz(State *state, double temperature);

  /**
   * Suggest a "reasonable" action.
   */
  Action *suggestAction(State *state);
  
  /**
   * Use what the agent has learned (no exploration).
   */
  Action *exploit(State *state);
   
private:
  /**
   * Computes the sigma sum - used for soft-max policy.
   */
  double computeSigma(State *state, double temperature);
  
  /**
   * Returns a "reasonable" temperature
   */
  double computeReasonableTemp();

  StateSpace *m_Q;            // Q-values
  StateSpace *m_numQ;         // Count the number of times we have visited each (s,a) 
  
  Action *m_suggestedAction;  // The action our brain suggests
  Action *m_tempAction;       // To avoid constant allocations
};

#endif
