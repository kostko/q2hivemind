/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "algebra.h"

namespace HiveMind {

namespace Algebra {

float yawFromVect(const Vector3f &delta)
{
  float yaw = atan2f(delta[1], delta[0]);
  if (yaw < 0)
    return 2*M_PI + yaw;
  
  return yaw;
}

float pitchFromVect(const Vector3f &delta)
{
  float pitch = atan2f(delta[2], sqrt(delta[0]*delta[0] + delta[1]*delta[1]));
  if (pitch < 0)
    return 2*M_PI + pitch;
  
  return pitch;
}

}

}


