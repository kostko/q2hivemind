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
  double yaw = atan2(delta[1], delta[0]);
  if (yaw < 0)
    return 2*M_PI + (float) yaw;
  
  return (float) yaw;
}

float pitchFromVect(const Vector3f &delta)
{
	float delta2;

	delta2=sqrt(delta[0]*delta[0]+delta[1]*delta[1]);
	if(delta2==0) {
		if(delta[2]>=0) {
			return M_PI/2;
		} else {
			return 3*M_PI/2;
		}
	} else {
		if(delta2>=0) {
			if(delta[2]>=0) {
				return (float)atan(delta[2]/delta2);
			} else {
				return 2*M_PI+(float)atan(delta[2]/delta2);
			}
		} else {
			return M_PI+(float)atan(delta[2]/delta2);
		}
	}
}

}

}


