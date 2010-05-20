/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_PLANNER_SENSORS_H
#define HM_PLANNER_SENSORS_H

#include "network/gamestate.h"

namespace HiveMind {

class Context;
class Map;

/**
 * A distance sensor enables the bot to sense distances to obstacles.
 */
class DistanceSensor {
public:
    // Parameters
    enum { step_size = 18 };
    
    /**
     * Class constructor. This constructs an invalid sensor.
     */
    DistanceSensor();
    
    /**
     * Class constructor.
     *
     * @param context Bot context
     * @param angle Sensor angle
     * @param distance Sensing distance (in steps)
     * @param noisy Should the sensor add random noise to angles
     */
    DistanceSensor(Context *context, float angle = 0.0, float distance = 5, bool noisy = true);
    
    /**
     * Updates the sensor.
     *
     * @param state Current game state
     * @param yaw Current yaw
     */
    void update(const GameState &state, float yaw);
    
    /**
     * Returns last sensor measurement. Note that update must be called
     * before any measurements can be optained.
     */
    inline float getMeasurement() const { return m_measure; }
    
    /**
     * Returns the angle against which the measurement has been performed.
     * This might differ from set angle in case artificial noise is
     * introduced.
     */
    inline float getMeasureAngle() const { return m_measureAngle; }
    
    /**
     * Returns true when this sensor has hit something.
     */
    inline bool isHit() const { return m_measure < m_senseDistance; }
    
    /**
     * Sets the sensor's angle.
     *
     * @param angle Angle in degrees
     */
    inline void setAngle(float angle) { m_angle = angle; }
    
    /**
     * Sets the sensor's max sensing distance.
     *
     * @param dist Sensing distance
     */
    inline void setSensingDistance(float dist) { m_senseDistance = dist; }
private:
    // Context and map instances
    Context *m_context;
    Map *m_map;
    
    // Sensor orientation angle and max sensing distance
    float m_senseDistance;
    float m_angle;
    float m_measureAngle;
    bool m_noisy;
    
    // Latest measurement
    float m_measure;
};

}

#endif

