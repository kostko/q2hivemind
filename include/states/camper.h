/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_CAMPER_H
#define HM_STATES_CAMPER_H

#include "planner/state.h"
#include "mapping/map.h"
#include "timing.h"

#include <list>
#include <boost/any.hpp>

namespace HiveMind {

class Context;

/**
 * Camper state.
 */
class CamperState : public State {
public:
    /**
     * Class constructor.
     *
     * @param context Bot context
     */
    CamperState(Context *context);
    
    /**
     * Class destructor.
     */
    virtual ~CamperState();
    
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
private:
    // When was this state entered
    timestamp_t m_lastEntered;

    bool m_alreadyStartedPoll;

    Vector3f m_lastLocation;
};

}

#endif

