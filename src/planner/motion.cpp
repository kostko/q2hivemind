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
  : m_context(context),
    m_yaw(0)
{
  Object::init();
  
  // Prepare sensors on the horizontal plane
  for (int i = 0; i < 10; i++) {
    m_sensors.push_back(DistanceSensor(context, 10.0f * (float) i));
    if (i > 0)
      m_sensors.push_back(DistanceSensor(context, -10.0f * (float) i));
  }
}

float MotionController::calculateMotion(const GameState &state, float yaw, float distance)
{
  // Turn towards our destination
  float divergence = 180.0f * (m_yaw - yaw) / M_PI;
  FuzzySeeker seeker(distance, divergence);
  if (m_yaw > yaw)
    m_yaw -= std::min(seeker.getCorrectionLimit(), m_yaw - yaw);
  else if (m_yaw < yaw)
    m_yaw += std::min(seeker.getCorrectionLimit(), yaw - m_yaw);
  
  // Perform an obstacle sensor sweep across the horizontal plane
  float distanceToObstacle = std::numeric_limits<float>::infinity();
  float angleToObstacle = 0.0;
  
  BOOST_FOREACH(DistanceSensor &sensor, m_sensors) {
    sensor.update(state, m_yaw);
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
  
  m_yaw += correction;
  if (m_yaw >= 2*M_PI)
    m_yaw = 2*M_PI - m_yaw;
  
  return m_yaw;
}

}


