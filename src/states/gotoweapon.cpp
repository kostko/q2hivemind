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
  m_items.push_back(ItemValue(Item::Shotgun, 1));
  m_items.push_back(ItemValue(Item::SuperShotgun, 1));
  m_items.push_back(ItemValue(Item::Machinegun, 1));
  m_items.push_back(ItemValue(Item::Chaingun, 1));
  m_items.push_back(ItemValue(Item::GrenadeLauncher, 1));
  m_items.push_back(ItemValue(Item::RocketLauncher, 1));
  m_items.push_back(ItemValue(Item::HyperBlaster, 1));
  m_items.push_back(ItemValue(Item::Railgun, 1));
  m_items.push_back(ItemValue(Item::BFG, 1));
}

void GoToWeaponState::evaluate()
{
  // TODO 
  m_items.sort(item_cmp);
}

}

