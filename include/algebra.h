/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_ALGEBRA_H
#define HM_ALGEBRA_H

#include "globals.h"

namespace HiveMind {

namespace Algebra {

float yawFromVect(const Vector3f &delta);

float pitchFromVect(const Vector3f &delta);

inline float randFloat()
{
  return rand() / (RAND_MAX + 1.0);
}

inline float randWeight()
{
  return randFloat() - randFloat();
}

inline int randInt(int x, int y)
{
  return rand() % (y - x + 1) + x;
}

}

}

#endif

