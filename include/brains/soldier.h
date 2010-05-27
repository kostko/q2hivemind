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

// Maximum number of frags
#define MAX_FRAGS 5    // TODO: get information from server

namespace HiveMind {

class State;

// Number of possible component values
enum StateComponentSize {
    N_ACTIONS = 6,
    N_STATE0 = 3,
    N_STATE1 = 9,
    N_STATE2 = 2,
    N_STATE3 = 2,
    N_STATE4 = MAX_FRAGS + 2
};

// Index to vector
enum ComponentToIndex {
    HEALTH,
    WEAPON,
    AMMO,
    ENEMY,
    FRAGS
};

// Action enumerator
enum Actions { 
    WANDER,
    SHOOT_AT,
    FIND_AMMO, 
    FIND_HEALTH, 
    FIND_BETTER_WEAPON,
    FIND_UPGRADE
};

// State component for health
enum State0  { 
    LOW_HEALTH, 
    MID_HEALTH, 
    HIGH_HEALTH 
};

// State component for weapon
enum State1 { 
    BLASTER, 
    SHOTGUN, 
    SUPER_SHOTGUN, 
    MACHINEGUN, 
    CHAINGUN, 
    GRENADE_LAUNCHER, 
    ROCKET_LAUNCHER,
    RAILGUN,
    BFG,
    HAND_GRENADES,
    HYPER_BLASTER
};

// State component for ammo
enum State2 {
    LOW_AMMO,
    HIGH_AMMO
};

// State component for enemy
enum State3 {
    VISIBLE,
    NOT_VISIBLE
};

// State component for frags
enum State4 {
    NEGATIVE_FRAGS = MAX_FRAGS + 1,
};

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

    /**
     * Init the brain.
     */
    void init();

    /**
     * Save the gained knowledge.
     */
    virtual void save();
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

    /**
     * Is the action possible?
     */
    virtual bool eligibleAction(BrainAction *action);

    /**
     * Returns the action that is always eligible.
     */
    virtual inline int alwaysEligibleId() { return WANDER; }

    /**
     * Get the default action name.
     */
    virtual std::string defaultActionName();

    // Weapon string to enum number map    
    boost::unordered_map<std::string, int> m_weaponMap;

    // Action ID to State map
    boost::unordered_map<int, State*> m_actionStateMap;
};

}

#endif
