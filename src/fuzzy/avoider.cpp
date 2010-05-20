/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "fuzzy/avoider.h"

#include <math.h>

namespace HiveMind {

FuzzyAvoider::FuzzyAvoider(float angle, float distance)
{
  Trapezoid angleSmall(-1, 0, 45, 60);
  Trapezoid angleMedium(55, 85, 95, 105);
  Trapezoid angleLarge(75, 110, 115, 180);
  Trapezoid distSmall(-1, 12, 25, 40);
  Trapezoid distMedium(30, 55, 65, 75);
  Trapezoid distLarge(70, 90, 100, 150);
  
  float w[9];
  angle = abs(angle);
  w[0] = angleSmall(angle) * distSmall(distance);
  w[1] = angleSmall(angle) * distMedium(distance);
  w[2] = angleSmall(angle) * distLarge(distance);
  w[3] = angleMedium(angle) * distSmall(distance);
  w[4] = angleMedium(angle) * distMedium(distance);
  w[5] = angleMedium(angle) * distLarge(distance);
  w[6] = angleLarge(angle) * distSmall(distance);
  w[7] = angleLarge(angle) * distMedium(distance);
  w[8] = angleLarge(angle) * distLarge(distance);
  
  float sum = w[0] + w[1] + w[2] + w[3] + w[4] + w[5] + w[6] + w[7] + w[8];
  m_result = (w[0] * 90) + (w[1] * 55) + (w[2] * 35) +
             (w[3] * 55) + (w[4] * 35) + (w[5] * 20) +
             (w[6] * 35) + (w[7] * 20) + (w[8] * 5);
  
  // Normalize result
  if (sum == 0.0f) {
    m_result = 0.0;
  } else {
    m_result = m_result / sum;
    m_result = (m_result / 180.0) * M_PI;
  }
}

}


