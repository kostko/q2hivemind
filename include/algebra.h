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

}

}

#endif

