/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_GOTOAMMO_H
#define	HM_STATES_GOTOAMMO_H

#include "states/goto.h"

namespace HiveMind {

class GoToAmmoState : public GoToState {
public:
    /**
     * Constructor.
     */
    GoToAmmoState(Context *context);
protected:
    /**
     * Evaluates the items based on the current needs.
     */
    virtual void evaluate();

    boost::unordered_map<std::string, int> m_weapons;
    boost::unordered_map<std::string, Item::Type> m_weaponsAmmo;
};
}

#endif

