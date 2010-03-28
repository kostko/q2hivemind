/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_NETWORK_GAMESTATE_H
#define HM_NETWORK_GAMESTATE_H

#include "globals.h"

#include <string>
#include <vector>

namespace HiveMind {

class Player {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  Vector3f angles;
  Vector3f origin;
  Vector3f velocity;
  Vector3f serverOrigin;
  
  int health;
  std::string ammoIcon;
  
  int ammo;
  std::string armorIcon;
  
  int armor;
  std::string weaponModel;
  
  int timer;
  int frags;
};

class Entity {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  inline bool isVisible() const { return m_updated; }
  
  void setVisible(bool visible) { m_updated = visible; }
public:
  Vector3f angles;
  Vector3f origin;
  Vector3f velocity;
  
  int modelIndex;
  int modelIndex2;
  int modelIndex3;
  int modelIndex4;
  
  int framenum;
  int renderfx;
private:
  bool m_updated;
};

class GameState {
public:
  Player player;
  int playerEntityId;
  int maxPlayers;
  Entity entities[1024]; 
};

class InternalPlayer {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  Vector3f angles;
  Vector3f origin;
  Vector3f velocity;
  
  int gunindex;
  int stats[32];
};

class InternalGameState {
public:
  int timestamp;
  InternalPlayer player;
  Entity entities[1024];
};

class TimePoint {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  int timestamp;
  Vector3f origin;
};

}

#endif

