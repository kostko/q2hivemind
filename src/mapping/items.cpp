/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mapping/items.h"

namespace HiveMind {

boost::unordered_map<std::string, Item::Type> Item::m_modelMap;

Item::Item(Type type)
  : m_type(type),
    m_weapon(type > WeaponStart),
    m_lastSeen(Timing::getCurrentTimestamp())
{
}

Item Item::forModel(const std::string &model)
{
  // Initialize model map when not available
  if (!m_modelMap.size()) {
    // Items
    m_modelMap["models/items/healing/medium/tris.md2"      ] = MediumHealth;
    m_modelMap["models/items/healing/large/tris.md2"       ] = LargeHealth;
    m_modelMap["models/items/healing/stimpack/tris.md2"    ] = StimPack;
    m_modelMap["models/items/mega_h/tris.md2"              ] = MegaHealth;
    m_modelMap["models/items/adrenal/tris.md2"             ] = Adrenaline;
    m_modelMap["models/items/ammo/bullets/medium/tris.md2" ] = Bullets;
    m_modelMap["models/items/ammo/cells/medium/tris.md2"   ] = Cells;
    m_modelMap["models/items/ammo/grenades/medium/tris.md2"] = Grenades;
    m_modelMap["models/items/ammo/rockets/medium/tris.md2" ] = Rockets;
    m_modelMap["models/items/ammo/shells/medium/tris.md2"  ] = Shells;
    m_modelMap["models/items/ammo/slugs/medium/tris.md2"   ] = Slugs;
    m_modelMap["models/items/armor/body/tris.md2"          ] = BodyArmor;
    m_modelMap["models/items/armor/combat/tris.md2"        ] = CombatArmor;
    m_modelMap["models/items/armor/jacket/tris.md2"        ] = JacketArmor;
    m_modelMap["models/items/armor/screen/tris.md2"        ] = PowerScreen;
    m_modelMap["models/items/armor/shard/tris.md2"         ] = ArmorShard;
    m_modelMap["models/items/armor/shield/tris.md2"        ] = PowerShield;
    m_modelMap["models/items/band/tris.md2"                ] = Bandolier;
    m_modelMap["models/items/invulner/tris.md2"            ] = Invulnerability;
    m_modelMap["models/items/pack/tris.md2"                ] = Backpack;
    m_modelMap["models/items/quaddama/tris.md2"            ] = Quad;
    m_modelMap["models/items/silencer/tris.md2"            ] = Silencer;
    
    // Weapons
    m_modelMap["models/weapons/g_shotg/tris.md2"           ] = Shotgun;
    m_modelMap["models/weapons/g_shotg2/tris.md2"          ] = SuperShotgun;
    m_modelMap["models/weapons/g_machn/tris.md2"           ] = Machinegun;
    m_modelMap["models/weapons/g_chain/tris.md2"           ] = Chaingun;
    m_modelMap["models/weapons/g_launch/tris.md2"          ] = GrenadeLauncher;
    m_modelMap["models/weapons/g_rocket/tris.md2"          ] = RocketLauncher;
    m_modelMap["models/weapons/g_hyperb/tris.md2"          ] = HyperBlaster;
    m_modelMap["models/weapons/g_rail/tris.md2"            ] = Railgun;
    m_modelMap["models/weapons/g_bfg/tris.md2"             ] = BFG;
  }
  
  return Item(m_modelMap.at(model));
}

}


