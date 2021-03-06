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
#include "mapping/grid.h"
#include "timing.h"

#include <list>
#include <boost/any.hpp>

namespace HiveMind {

class Context;
class GoToState;

/**
 * Wander state.
 */
class WanderState : public State {
friend class GoToState;
friend class DropWeaponState;
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
     * @param restored True if state was restored from stack
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
     * Special constructor for subclasses.
     */
    WanderState(Context *context, const std::string &name, int eligibilityTime, bool prunable);

    /**
     * Returns the distance from origin to destination.
     */
    float getDistanceToDestination() const;
    
    /**
     * Resets point-dependent statistics.
     */
    void resetPointStatistics();
    
    /**
     * Requests path recomputation from current location to our
     * destination.
     *
     * @param randomize True means to pick the next node at random
     */
    void recomputePath(bool randomize = false);

    // Have we reached the destination?
    bool m_atDestination;
private:
    // Current path to follow
    GridPath m_currentPath;
    bool m_hasNextPoint;
    timestamp_t m_lastFrameUpdate;
    float m_speed;
    
    // Metadata used for travel checks
    float m_minDistance;
    float m_maxDistance;
    float m_lastZ;
    timestamp_t m_lastMinChange;
    Vector3f m_lastOrigin;
    bool m_randomize;
};

}

#endif

