/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_FUZZY_TRAPEZOID_H
#define HM_FUZZY_TRAPEZOID_H

#include <algorithm>

namespace HiveMind {

/**
 * A simple trapezoid representation needed for fuzzy logic.
 */
class Trapezoid {
public:
    /**
     * Class constructor.
     */
    Trapezoid(float a, float b, float c, float d)
      : a(a), b(b), c(c), d(d)
    {
    }
    
    /**
     * Computes contribution of this trapezoid area at a specific
     * point.
     *
     * @param input Point location
     * @return Trapezoid contribution
     */ 
    inline float operator()(float input) const
    {
      float tmp1 = (input - a) / (b - a);
      float tmp2 = (d - input) / (d - c);
      
      tmp1 = std::min(tmp1, std::min(1.0f, tmp2));
      return std::max(tmp1, 0.0f);
    }
private:
    // Points describing the trapezoid
    float a, b, c, d;
};

}

#endif

