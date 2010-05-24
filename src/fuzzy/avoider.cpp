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
  // Fuzzy angles
  Trapezoid angleSmall(-1, 0, 25, 35);
  Trapezoid angleMedium(30, 45, 60, 70);
  Trapezoid angleLarge(40, 75, 88, 100);
  
  // Fuzzy distances
  Trapezoid distSmall(-1, 12, 25, 40);
  Trapezoid distMedium(30, 55, 65, 75);
  Trapezoid distLarge(70, 90, 100, 150);
  
  float w[9];
  angle = fabsf(angle);
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
  m_result = (w[0] * 15) + (w[1] * 7) + (w[2] * 4) +
             (w[3] * 7) + (w[4] * 4) + (w[5] * 2) +
             (w[6] * 4) + (w[7] * 2) + (w[8] * 1);
  
  // Normalize result
  if (sum == 0.0f) {
    m_result = 0.0;
  } else {
    m_result = m_result / sum;
    m_result = (m_result / 180.0) * M_PI;
  }
}

FuzzySeeker::FuzzySeeker(float distance, float divergence)
{
  // Fuzzy angle divergences
  Trapezoid divSmall(-1, 0, 10, 20);
  Trapezoid divMedium(15, 30, 45, 55);
  Trapezoid divLarge(40, 70, 110, 185);
  
  // Fuzzy distances
  Trapezoid distSmall(-1, 12, 25, 40);
  Trapezoid distMedium(30, 55, 65, 75);
  Trapezoid distLarge(70, 90, 100, 150);
  
  float w[9];
  divergence = std::min(180.0f, fabsf(divergence));
  distance = std::min(145.0f, fabsf(distance));
  w[0] = distSmall(distance) * divLarge(divergence);
  w[1] = distSmall(distance) * divMedium(divergence);
  w[2] = distSmall(distance) * divSmall(divergence);
  w[3] = distMedium(distance) * divLarge(divergence);
  w[4] = distMedium(distance) * divMedium(divergence);
  w[5] = distMedium(distance) * divSmall(divergence);
  w[6] = distLarge(distance) * divLarge(divergence);
  w[7] = distLarge(distance) * divMedium(divergence);
  w[8] = distLarge(distance) * divSmall(divergence);
  
  float sum = w[0] + w[1] + w[2] + w[3] + w[4] + w[5] + w[6] + w[7] + w[8];
  m_result = (w[0] * 90) + (w[1] * 45) + (w[2] * 25) +
             (w[3] * 45) + (w[4] * 25) + (w[5] * 10) +
             (w[6] * 25) + (w[7] * 10) + (w[8] * 5);

  // Normalize result
  if (sum == 0.0f) {
    m_result = 0.0;
  } else {
    m_result = m_result / sum;
    m_result = (m_result / 180.0) * M_PI;
  }
}

}


