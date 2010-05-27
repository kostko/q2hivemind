/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_GOTOWEAPON_H
#define	HM_STATES_GOTOWEAPON_H

#include "states/goto.h"

namespace HiveMind {

class GoToWeaponState : public GoToState {
public:
    /**
     * Constructor.
     */
    GoToWeaponState(Context *context);

    /**
     * Evaluates the items based on the current needs.
     */
    virtual void evaluate();
private:
    boost::unordered_map<Item, std::string> m_itemStringMap;
};

}

#endif

