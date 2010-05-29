/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_DROP_H
#define HM_STATES_DROP_H

#include "planner/state.h"
#include "mapping/map.h"
#include "states/wander.h"
#include "timing.h"

#include <list>
#include <boost/any.hpp>

namespace HiveMind {

class Context;
class GoToAndDropWeaponEvent;

/**
 * Drop weapon state.
 */
class DropWeaponState : public WanderState {
public:
    /**
     * Class constructor.
     *
     * @param context Bot context
     */
    DropWeaponState(Context *context);

    /**
     * Class destructor.
     */
    virtual ~DropWeaponState();

    /**
     * This method gets called when a bot has been chosen to come and drop a weapon.
     */
    void dropOrderReceived(GoToAndDropWeaponEvent *event);

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
     * This method should implement state specific processing in
     * planning mode. This method is called in planner thread
     * context.
     */
    virtual void processPlanning();

    /**
     * This method should implement state specific processing on
     * each frame update. This method is called in main thread
     * context.
     */
    virtual void processFrame();

    void dropWeapon();
private:
    Vector3f m_dropLocation;
    int m_i;
};

}

#endif

