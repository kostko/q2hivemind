/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_SHOOT_H
#define	HM_STATES_SHOOT_H

#include "timing.h"
#include "planner/state.h"

namespace HiveMind {

class Context;
class OpponentSpottedEvent;

enum {
    NO_ENEMY = -1,
    MIN_DISTANCE = 500,      // The minimum distance for spotting an enemy
    MIN_SHOOT_TIME = 1000     // Minimum shoot time
};

// Offsets for ray testing if the enemy is alive
const float ENEMY_OFFSETS[] = { 0.0, -16.0, 16.0, 32.0 };

/**
 * Shoot state.
 */
class ShootState : public State {
public:
    /**
     * Class constructor.
     *
     * @param context Bot context
     */
    ShootState(Context *context);

    /**
     * Class destructor.
     */
    virtual ~ShootState();

    /**
     * Prepare for entry into this state.
     *
     * @param metadata Supplied metadata
     */
    virtual void initialize(const boost::any &metadata);

    /**
     * Prepare for leaving this state.
     */
    virtual void goodbye();

    /**
     * This method should implement state specific processing on
     * each frame update. This method is called in main thread
     * context.
     */
    virtual void processFrame();

    /**
     * This method should implement state specific event
     * checking, so the state can emit a signal when
     * needed. This method is called in main thread context.
     */
    virtual void checkEvent();

    /**
     * Make this state one of the candidates for transition. This method is called in main thread context.
     */
    void makeEligible(OpponentSpottedEvent *event);
private:
    /**
     *  Returns the entity ID of the closest enemy.
     */
    int getClosestEnemy();

    // Entity to shoot at
    int m_targetId;

    // Time of entry into shoot state
    timestamp_t m_shootStart;
};

}

#endif
