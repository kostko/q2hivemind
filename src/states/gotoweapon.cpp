/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/gotoweapon.h"
#include "mapping/items.h"

namespace HiveMind {

GoToWeaponState::GoToWeaponState(Context *context)
  : GoToState(context, "gotoweapon")
{
  // At the beginning all items are equally valuable
  m_items.push_back(ItemValue(Item::Shotgun, 2));
  m_items.push_back(ItemValue(Item::SuperShotgun, 3));
  m_items.push_back(ItemValue(Item::Machinegun, 4));
  m_items.push_back(ItemValue(Item::Chaingun, 5));
  m_items.push_back(ItemValue(Item::GrenadeLauncher, 6));
  m_items.push_back(ItemValue(Item::RocketLauncher, 7));
  m_items.push_back(ItemValue(Item::HyperBlaster, 8));
  m_items.push_back(ItemValue(Item::Railgun, 9));
  m_items.push_back(ItemValue(Item::BFG, 10));
}

void GoToWeaponState::evaluate()
{
}

}

