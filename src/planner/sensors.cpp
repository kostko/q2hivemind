/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/sensors.h"
#include "context.h"
#include "mapping/map.h"
#include "algebra.h"

#include <Eigen/Geometry>

using namespace Eigen;

namespace HiveMind {

DistanceSensor::DistanceSensor()
  : m_context(NULL),
    m_map(NULL),
    m_angle(0),
    m_measureAngle(0),
    m_senseDistance(0.0),
    m_measure(0.0),
    m_noisy(true)
{
}

DistanceSensor::DistanceSensor(Context *context, float angle, float distance, bool noisy)
  : m_context(context),
    m_map(context->getMap()),
    m_angle(angle),
    m_measureAngle(angle),
    m_senseDistance(distance),
    m_measure(distance),
    m_noisy(noisy)
{
}

void DistanceSensor::update(const GameState &state, float yaw)
{
  if (m_map == NULL || m_context == NULL)
    return;
  
  // When the sensor is noisy we should perturb the angle a bit
  float angle = m_angle;
  if (m_noisy) {
    angle += Algebra::randWeight() * 3.0;
    m_measureAngle = angle;
  }
  
  Vector3f origin = state.player.origin;
  Quaternionf rotation(AngleAxis<float>((angle / 180.0) * M_PI + yaw, Vector3f::UnitZ()));
  Vector3f p = rotation * Vector3f::UnitX();
  p[2] = 0;
  p.normalize();
  
  // Now simulate a walk so we handle stairs and detect gaps
  float distance = 0.0;
  Vector3f dest = origin;
  p *= (float) step_size;
  
  // Step up the first stair
  dest[2] += step_size;
  float d = m_map->rayTest(origin, dest, Map::Solid);
  origin += (dest - origin) * d;
  dest = origin;
  
  while (distance < m_senseDistance) {
    dest = origin + p;
    
    // Trace to nearest solid
    // TODO handle dynamic entities here (players)
    d = m_map->rayTest(origin, dest, Map::Solid);
    if (d < 1.0f) {
      distance += d;
      dest = origin + (dest - origin) * d;
      break;
    }
    
    origin += (dest - origin) * d;
    dest = origin;
    
    // Check for gaps or walk down stairs
    dest[2] -= 4*step_size;
    d = m_map->rayTest(origin, dest, Map::Solid);
    if (d == 1.0f) {
      dest = origin;
      break;
    }
    
    origin += (dest - origin) * d;
    dest = origin;
    
    // Automatically step up stairs
    dest[2] += step_size;
    d = m_map->rayTest(origin, dest, Map::Solid);
    origin += (dest - origin) * d;
    dest = origin;
    
    // Increase distance
    distance += 1;
  }
  
  // Compute distance to obstacle
  m_measure = (dest - state.player.origin).norm(); 
}

}


