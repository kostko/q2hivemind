/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_GOTOHEALTH_H
#define	HM_STATES_GOTOHEALTH_H

#include "states/goto.h"

namespace HiveMind {

class GoToHealthState : public GoToState {
public:
    /**
     * Constructor.
     */
    GoToHealthState(Context *context);

    /**
     * Evaluates the items based on the current needs.
     */
    virtual void evaluate();
};

}

#endif

