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

#include <Eigen/Geometry>

// XXX
#include <iostream>

using namespace Eigen;

namespace HiveMind {

DistanceSensor::DistanceSensor()
  : m_context(NULL),
    m_map(NULL),
    m_angle(0),
    m_senseDistance(0.0),
    m_measure(0.0)
{
}

DistanceSensor::DistanceSensor(Context *context)
  : m_context(context),
    m_map(context->getMap()),
    m_angle(0),
    m_senseDistance(50.0),
    m_measure(50.0)
{
}

void DistanceSensor::update(const GameState &state, float yaw)
{
  if (m_map == NULL || m_context == NULL)
    return;
  
  Vector3f origin = state.player.origin;
  Quaternionf rotation(AngleAxis<float>((m_angle / 180.0) * M_PI + yaw, Vector3f::UnitZ()));
  Vector3f p = rotation * Vector3f::UnitX();
  p = origin + p.normalized() * m_senseDistance;
  
  // Now that we have both points perform a simple ray test
  m_measure = m_map->rayTest(origin, p, Map::Solid) * m_senseDistance;
}

}


