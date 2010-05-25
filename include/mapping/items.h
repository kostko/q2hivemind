/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MAPPING_ITEMS_H
#define HM_MAPPING_ITEMS_H

#include "globals.h"
#include "timing.h"

namespace HiveMind {

/**
 * Represents a collectable item.
 */
class Item {
public:
    /**
     * Valid item types.
     */
    enum Type {
      // Standard items
      MediumHealth,
      LargeHealth,
      StimPack,
      MegaHealth,
      Adrenaline,
      Bullets,
      Cells,
      Grenades,
      Rockets,
      Shells,
      Slugs,
      BodyArmor,
      CombatArmor,
      JacketArmor,
      PowerScreen,
      ArmorShard,
      PowerShield,
      Bandolier,
      Invulnerability,
      Backpack,
      Quad,
      Silencer,
      
      // Weapon start marker
      WeaponStart,
      
      // Weapons
      Shotgun,
      SuperShotgun,
      Machinegun,
      Chaingun,
      GrenadeLauncher,
      RocketLauncher,
      HyperBlaster,
      Railgun,
      BFG
    };
    
    /**
     * Class constructor.
     *
     * @param type Item type
     */
    Item(Type type);
    
    /**
     * Returns item's type.
     */
    inline Type getType() const { return m_type; }
    
    /**
     * Sets item's location.
     */
    inline void setLocation(const Vector3f &location) { m_location = location; }
    
    /**
     * Returns item's location.
     */
    inline Vector3f getLocation() const { return m_location; }
    
    /**
     * Returns true if this item is a weapon.
     */
    inline bool isWeapon() const { return m_weapon; }
    
    /**
     * Updates the last seen timestamp to current time.
     */
    inline void updateLastSeen() { m_lastSeen = Timing::getCurrentTimestamp(); }
    
    /**
     * Returns the timestamp when this item has last been seen on
     * this location.
     */
    inline timestamp_t getLastSeen() const { return m_lastSeen; }
    
    /**
     * Returns a new item of proper type.
     *
     * @param model Model string
     */
    static Item forModel(const std::string &model);
    
    /**
     * Comparison operator.
     */
    inline bool operator==(const Item &other) const
    {
      return m_type == other.m_type && m_location == other.m_location;
    }
    
    /**
     * Hash function.
     */
    friend std::size_t hash_value(const Item &item)
    {
      std::size_t seed = 0;
      boost::hash_combine(seed, item.m_type);
      boost::hash_combine(seed, item.m_location[0]);
      boost::hash_combine(seed, item.m_location[1]);
      boost::hash_combine(seed, item.m_location[2]);
      return seed;
    }
private:
    // Item properties
    Type m_type;
    Vector3f m_location;
    bool m_weapon;
    timestamp_t m_lastSeen;
    
    // Item map
    static boost::unordered_map<std::string, Type> m_modelMap;
};

}

#endif

