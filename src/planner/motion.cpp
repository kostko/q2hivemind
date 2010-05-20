/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/motion.h"
#include "planner/local.h"
#include "context.h"
#include "logger.h"
#include "fuzzy/avoider.h"

#include <algorithm>
#include <limits>
#include <boost/foreach.hpp>

namespace HiveMind {

class Context;

MotionController::MotionController(Context *context)
  : m_context(context)
{
  Object::init();
  
  // Prepare sensors on the horizontal plane
  m_sensors.push_back(DistanceSensor(context, 75.0));
  m_sensors.push_back(DistanceSensor(context, 45.0));
  m_sensors.push_back(DistanceSensor(context, 25.0));
  m_sensors.push_back(DistanceSensor(context, 0.0));
  m_sensors.push_back(DistanceSensor(context, -25.0));
  m_sensors.push_back(DistanceSensor(context, -45.0));
  m_sensors.push_back(DistanceSensor(context, -75.0));
}

float MotionController::calculateMotion(const GameState &state, float yaw)
{
  // Perform an obstacle sensor sweep across the horizontal plane
  float distanceToObstacle = std::numeric_limits<float>::infinity();
  float angleToObstacle = 0.0;
  
  BOOST_FOREACH(DistanceSensor &sensor, m_sensors) {
    sensor.update(state, yaw);
    if (sensor.getMeasurement() < distanceToObstacle) {
      distanceToObstacle = sensor.getMeasurement();
      angleToObstacle = sensor.getMeasureAngle();
    }
  }
  
  // Calculate change using fuzzy logic
  FuzzyAvoider avoider(angleToObstacle, distanceToObstacle);
  float correction = avoider.getAngleCorrection();
  if (angleToObstacle > 0.0)
    correction = -correction;
  
  return yaw + correction;
}

}


