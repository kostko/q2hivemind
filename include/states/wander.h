/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_WANDER_H
#define HM_STATES_WANDER_H

#include "planner/state.h"
#include "mapping/map.h"
#include "timing.h"

#include <list>
#include <boost/any.hpp>

namespace HiveMind {

class Context;

/**
 * Wander state.
 */
class WanderState : public State {
public:
    /**
     * Class constructor.
     *
     * @param context Bot context
     */
    WanderState(Context *context);
    
    /**
     * Class destructor.
     */
    virtual ~WanderState();
    
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
     * This method should implement state specific processing in
     * planning mode. This method is called in planner thread
     * context.
     */
    virtual void processPlanning();
    
protected:
    /**
     * Returns the current destination vector.
     */
    Vector3f getNextDestination() const;
    
    /**
     * Returns the distance from origin to destination.
     */
    float getDistanceToDestination() const;
    
    /**
     * Sets new destination point on current path.
     */
    void travelToPoint(int index);
    
    /**
     * Returns the current link.
     */
    MapLink *getCurrentLink() const;
    
    /**
     * Requests path recomputation from current location to our
     * destination.
     *
     * @param markInvalidOnNone True if planner should mark the link
     *                          that lead here as invalid when no path
     *                          out can be found
     */
    void recomputePath(bool markInvalidOnNone = false);
private:
    // Current path to follow
    MapPath m_currentPath;
    int m_nextPoint;
    timestamp_t m_lastFrameUpdate;
    float m_speed;
    
    // Metadata used for travel checks
    float m_minDistance;
    float m_maxDistance;
    float m_lastZ;
    timestamp_t m_lastMinChange;
    
    // For checking invalid links
    bool m_markInvalidOnNone;
    MapLink *m_lastLink;
    
    // State completed flag
    bool m_complete;
};

}

#endif

