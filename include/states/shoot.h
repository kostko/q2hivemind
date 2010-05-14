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

enum {
    NO_ENEMY = -1,
    MIN_DISTANCE = 500,      // The minimum distance for spotting an enemy
    MIN_SHOOT_TIME = 700     // Minimum shoot time
};

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
    virtual void initialize(const boost::any &metadata, bool restored);

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
     * This method should implement state specific processing in
     * planning mode. This method is called in planner thread
     * context.
     */
    virtual void processPlanning();
    
    virtual void checkInterruption();
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