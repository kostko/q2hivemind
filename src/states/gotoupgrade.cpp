/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/gotoupgrade.h"
#include "mapping/items.h"

namespace HiveMind {

GoToUpgradeState::GoToUpgradeState(Context *context)
  : GoToState(context, "gotoupgrade")
{
  // At the beginning all items are equally valuable
  m_items.push_back(ItemValue(Item::BodyArmor, 1));
  m_items.push_back(ItemValue(Item::CombatArmor, 2));
  m_items.push_back(ItemValue(Item::JacketArmor, 3));
  m_items.push_back(ItemValue(Item::PowerScreen, 4));
  m_items.push_back(ItemValue(Item::ArmorShard, 5));
  m_items.push_back(ItemValue(Item::PowerShield, 6));
  m_items.push_back(ItemValue(Item::Bandolier, 1));
  m_items.push_back(ItemValue(Item::Invulnerability, 10));
  m_items.push_back(ItemValue(Item::Backpack, 1));
  m_items.push_back(ItemValue(Item::Quad, 10));
  m_items.push_back(ItemValue(Item::Silencer, 1));
}

void GoToUpgradeState::evaluate()
{
}

}

