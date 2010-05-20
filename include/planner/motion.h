/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_PLANNER_MOTION_CONTROLLER_H
#define HM_PLANNER_MOTION_CONTROLLER_H

#include "network/gamestate.h"
#include "planner/sensors.h"
#include "eann/behaviour.h"
#include "timing.h"
#include "object.h"

#include <vector>

namespace HiveMind {

class Context;

/**
 * Bot motion controller.
 */
class MotionController : public Object {
public:
    /**
     * Class constructor.
     *
     * @param context Context instance
     */
    MotionController(Context *context);
    
    /**
     * Calculates the output yaw given the wanted direction and
     * game state.
     *
     * @param state Current gamestate
     * @param yaw Yaw in radians
     * @return Corrected yaw to use
     */
    float calculateMotion(const GameState &state, float yaw);
private:
    // Context
    Context *m_context;
    
    // Sensors
    std::vector<DistanceSensor> m_sensors;
};

}

#endif

