/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/gotohealth.h"
#include "mapping/items.h"

namespace HiveMind {

GoToHealthState::GoToHealthState(Context *context)
  : GoToState(context, "gotohealth")
{
  // At the beginning all items are equally valuable
  m_items.push_back(ItemValue(Item::MediumHealth, 1));
  m_items.push_back(ItemValue(Item::LargeHealth, 1));
  m_items.push_back(ItemValue(Item::StimPack, 1));
  m_items.push_back(ItemValue(Item::MegaHealth, 1));
}

void GoToHealthState::evaluate()
{
  // TODO give higher values to the weapons in our inventory
  m_items.sort(item_cmp);
}

}
