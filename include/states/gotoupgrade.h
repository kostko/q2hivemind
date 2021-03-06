/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_GOTOUPGRADE_H
#define	HM_STATES_GOTOUPGRADE_H

#include "states/goto.h"

namespace HiveMind {

class GoToUpgradeState : public GoToState {
public:
    /**
     * Constructor.
     */
    GoToUpgradeState(Context *context);

    /**
     * Evaluates the items based on the current needs.
     */
    virtual void evaluate();
};

}

#endif

