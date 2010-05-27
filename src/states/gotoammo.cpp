/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/gotoammo.h"
#include "mapping/items.h"
#include "planner/local.h"

#include <boost/foreach.hpp>

namespace HiveMind {

GoToAmmoState::GoToAmmoState(Context *context)
  : GoToState(context, "gotoammo")
{
  m_weapons["Grenades"] = 0;
  m_weapons["Blaster"] = 1;
  m_weapons["Shotgun"] = 2;
  m_weapons["Super Shotgun"] = 3;
  m_weapons["Machinegun"] = 4;
  m_weapons["Chaingun"] = 5;
  m_weapons["Grenade Launcher"] = 6;
  m_weapons["Rocket Launcher"] = 7;
  m_weapons["HyperBlaster"] = 8;
  m_weapons["Railgun"] = 9;
  m_weapons["BFG10K"] = 10;

  m_weaponsAmmo["Shotgun"] = Item::Shells;
  m_weaponsAmmo["Super Shotgun"] = Item::Shells;
  m_weaponsAmmo["Machinegun"] = Item::Bullets;
  m_weaponsAmmo["Chaingun"] = Item::Bullets;
  m_weaponsAmmo["Grenade Launcher"] =  Item::Grenades;
  m_weaponsAmmo["Rocket Launcher"] = Item::Rockets;
  m_weaponsAmmo["HyperBlaster"] = Item::Cells;
  m_weaponsAmmo["Railgun"] = Item::Slugs;
  m_weaponsAmmo["BFG10K"] = Item::Cells;
}

void GoToAmmoState::evaluate()
{
  // All ammo equally valued if we only have a blaster
  if (getLocalPlanner()->bestWeaponInInventory("") == "Blaster") {
    m_items.push_back(ItemValue(Item::Bullets, 1));
    m_items.push_back(ItemValue(Item::Cells, 1));
    m_items.push_back(ItemValue(Item::Grenades, 1));
    m_items.push_back(ItemValue(Item::Rockets, 1));
    m_items.push_back(ItemValue(Item::Shells, 1));
    m_items.push_back(ItemValue(Item::Slugs, 1));
    return;
  }

  m_items.clear();
  typedef std::pair<std::string, int> InventoryPair;
  BOOST_FOREACH(InventoryPair element, m_gameState->inventory) {
    std::string w = element.first;

    if (m_weapons.find(w) == m_weapons.end())
      continue;

    if (w == "Blaster")
      continue;
    
    // Assign the proper values for the ammo we actually need
    Item::Type ammo = m_weaponsAmmo[w];
    m_items.push_back(ItemValue(ammo, m_weapons[w]));
  }

  m_items.sort(item_cmp);
}

}
