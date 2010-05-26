/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/gotoammo.h"
#include "mapping/items.h"

namespace HiveMind {

GoToAmmoState::GoToAmmoState(Context *context)
  : GoToState(context, "gotoammo")
{
  // At the beginning all items are equally valuable
  m_items.push_back(ItemValue(Item::Bullets, 1));
  m_items.push_back(ItemValue(Item::Cells, 1));
  m_items.push_back(ItemValue(Item::Grenades, 1));
  m_items.push_back(ItemValue(Item::Rockets, 1));
  m_items.push_back(ItemValue(Item::Shells, 1));
  m_items.push_back(ItemValue(Item::Slugs, 1));
}

void GoToAmmoState::evaluate()
{
  // TODO give higher values to the weapons in our inventory
  m_items.sort(item_cmp);
}

}
