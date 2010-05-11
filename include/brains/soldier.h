/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_BRAINS_SOLDIER_H
#define HM_BRAINS_SOLDIER_H

#include "rl/brains.h"

namespace HiveMind {

/**
 * Implements a basic brain behaviour.
 */
class SoldierBrains : public Brains {
public:
    /**
     * Constructor.
     */
    SoldierBrains(LocalPlanner *planner);
    
    /**
     * Destructor.
     */
    ~SoldierBrains();

private:
    /**
     * What really defines the "brain" is the reward function.
     */
    virtual double reward(BrainState *prevState, BrainState *currState);
    
    /**
     * Observe my current state.
     */
    virtual BrainState *observe();
    
    /**
     * Execute the given action.
     */
    virtual void execute(BrainAction *action);
};

}

#endif
