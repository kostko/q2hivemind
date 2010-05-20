/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_FUZZY_AVOIDER_H
#define HM_FUZZY_AVOIDER_H

#include "fuzzy/trapezoid.h"

namespace HiveMind {

/**
 * A simple collision avoider made using fuzzy logic.
 */
class FuzzyAvoider {
public:
    /**
     * Class constructor.
     *
     * @param angle Angle to obstacle
     * @param distance Distance to obstacle
     */
    FuzzyAvoider(float angle, float distance);
    
    /**
     * Returns the required angle correction.
     */
    inline float getAngleCorrection() const { return m_result; }
private:
    // Resulting angle correction
    float m_result;
};

}

#endif

