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

#include "rl/statespace.h"
#include "object.h"

namespace HiveMind {

class LocalPlanner;

/**
 * Some constants of the Boltzmann "soft-max" control policy.
 */
#define GAMMA              0.6
#define MAX_Q_TEMPERATURE  1.0 / 2
#define MIN_Q_TEMPERATURE  1.0 / 50
#define CEILING            1000

/**
 * Random -> [0,1).
 */
#define random() ((double) rand()) / RAND_MAX

/**
 * Represents the "brains" of an agent.
 *
 * Concept taken from: http://www.compapp.dcu.ie/~humphrys/Notes/RL/Code/index.html
 */ 
class Brains : public Object {
public:
    /**
     * Constructor.
     */
    Brains(HiveMind::LocalPlanner *planner);

    /**
     * Destructor.
     */
    ~Brains();
    
    void init(vector<int> &stateComponents, vector<int> &actionComponents);
    
    /**
     * Brain-specific initialization.
     */
    virtual void init() = 0;

    /**
     * Interact with the world (selects the next action) and learns from it.
     */
    void interact();
    
    /**
     * Set brain mode.
     */
    void setBrainMode(bool learn);
    
    /**
     * State space.
     */
    StateSpace *getQ();
    
    /**
     * Get new empty BrainState object.
     */
    BrainState *newBrainState(std::string name = "");
    
    /**
     * Get new empty BrainAction object.
     */
    BrainAction *newBrainAction(std::string name = "");
          
protected:
    /**
     * What really defines the "brain" is the reward function.
     */
    virtual double reward(BrainState *prevState, BrainState *currState) = 0;
    
    /**
     * Observe my current state.
     */
    virtual BrainState *observe() = 0;
    
    /**
     * Execute the given action.
     */
    virtual void execute(BrainAction *action) = 0;

    /**
     * Is the action possible?
     */
    virtual bool eligibleAction(BrainAction *action) = 0;

    /**
     * Returns the action that is always eligible.
     */
    virtual int alwaysEligibleId() = 0;

    /**
     * Default action name.
     */
    virtual std::string defaultActionName() = 0;

    LocalPlanner *m_localPlanner;
    
    BrainAction *m_currAction;
    BrainAction *m_tempAction;       // To avoid constant allocations

    BrainState *m_currState;
    BrainState *m_tempState;         // To avoid constant allocations
private:
    /**
     * Update Q function.
     */ 
    void updateQ(BrainState *prevState, BrainAction *action, BrainState *currState);
    
    /**
     * Suggests a random action.
     */
    BrainAction *randomAction();
    
    /**
     * Suggest an action using the soft-max policy.
     */
    BrainAction *suggestBoltz(BrainState *state, double temperature);

    /**
     * Suggest a "reasonable" action.
     */
    BrainAction *suggestAction(BrainState *state);
    
    /**
     * Use what the agent has learned (no exploration).
     */
    BrainAction *exploit(BrainState *state);

    /**
     * Computes the sigma sum - used for soft-max policy.
     */
    double computeSigma(BrainState *state, double temperature);
    
    /**
     * Returns a "reasonable" temperature
     */
    double computeReasonableTemp();

    bool m_learn;               // Brain mode
    
    StateSpace *m_Q;            // Q-values
    StateSpace *m_numQ;         // Count the number of times we have visited each (s,a) 
    
    BrainAction *m_suggestedAction;  // The action our brain suggest
};

}

#endif
