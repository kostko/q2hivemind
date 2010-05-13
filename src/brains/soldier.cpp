/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "logger.h"
#include "planner/local.h"
#include "brains/soldier.h"
 
namespace HiveMind {

SoldierBrains::SoldierBrains(HiveMind::LocalPlanner *planner)
  : Brains(planner)
{
  vector<int> stateComps;
  stateComps.push_back(N_STATE0);
  stateComps.push_back(N_STATE1);
  stateComps.push_back(N_STATE2);
  stateComps.push_back(N_STATE3);
  stateComps.push_back(N_STATE4);
  
  vector<int> actionComps;
  actionComps.push_back(N_ACTIONS);
  
  Brains::init(stateComps, actionComps);
  
  std::string head = "models/weapons/";
  std::string tail = "/tris.md2";
  
  m_weaponMap[head + "v_blast" + tail]  = BLASTER;
  m_weaponMap[head + "v_chain" + tail]  = CHAINGUN;
  m_weaponMap[head + "v_bfg" + tail]    = BFG;
  m_weaponMap[head + "v_handgr" + tail] = HAND_GRENADES;
  m_weaponMap[head + "v_hyperb" + tail] = HYPER_BLASTER;
  m_weaponMap[head + "v_launch" + tail] = GRENADE_LAUNCHER;
  m_weaponMap[head + "v_machn" + tail]  = MACHINEGUN;
  m_weaponMap[head + "v_rail" + tail]   = RAILGUN;
  m_weaponMap[head + "v_shotg" + tail]  = SHOTGUN;
  m_weaponMap[head + "v_blast2" + tail] = SUPER_SHOTGUN;
  m_weaponMap[head + "v_rocket" + tail] = ROCKET_LAUNCHER;
  
  getLogger()->info("Brains initialized.");
}

SoldierBrains::~SoldierBrains()
{
}

double SoldierBrains::reward(BrainState *prevState, BrainState *currState)
{
  double reward = 0;
  
  // Small reward for health and ammo increase
  int dHealth = (*prevState)[HEALTH] - (*currState)[HEALTH];
  if (dHealth < 0)
    reward += 1;
    
  int dAmmo = (*prevState)[AMMO] - (*currState)[AMMO];
  if (dAmmo < 0)
    reward += 1;

  // Big reward for a frag
  int dFrags = (*prevState)[FRAGS] - (*currState)[FRAGS];
  if (dFrags < 0) 
    reward += 5;
  
  return reward;
}

BrainState *SoldierBrains::observe()
{
  GameState *gs = m_localPlanner->gameState();
  
  // Health
  int health = gs->player.health;
  if (health < 20) {    
    (*m_tempState)[HEALTH] = LOW_HEALTH;
  } else if (health > 50) {
    (*m_tempState)[HEALTH] = HIGH_HEALTH;
  } else {
    (*m_tempState)[HEALTH] = MID_HEALTH;
  }

  // Weapon
  int weapon = m_weaponMap[ gs->player.weaponModel ];
  (*m_tempState)[WEAPON] = weapon;
  
  // Ammo
  int ammo = gs->player.ammo;
  
  // The level of ammo depends on the weapon
  switch (weapon) {
    case BLASTER:
      (*m_tempState)[AMMO] = HIGH_AMMO;   // The blaster has infinite ammo...     
      break;
    case SHOTGUN:
    case SUPER_SHOTGUN:
      (*m_tempState)[AMMO] = ammo > 10 ? HIGH_AMMO : LOW_AMMO;  // 100 shells is the maximum for shotguns
      break;
   case MACHINEGUN:
   case CHAINGUN:
   case HYPER_BLASTER:
     (*m_tempState)[AMMO] = ammo > 50 ? HIGH_AMMO : LOW_AMMO;  // 200 bullets is the maximum for machineguns     
     break;
   case ROCKET_LAUNCHER:
   case GRENADE_LAUNCHER:
   case HAND_GRENADES:
     (*m_tempState)[AMMO] = ammo > 5 ? HIGH_AMMO : LOW_AMMO;  // 50 grenades is the maximum     
     break;
   case RAILGUN:
     (*m_tempState)[AMMO] = ammo > 5 ? HIGH_AMMO : LOW_AMMO;  // 50 grenades is the maximum
     break;
   case BFG:
     (*m_tempState)[AMMO] = ammo > 5 ? HIGH_AMMO : LOW_AMMO;  // although 200 is the maximum for BFG :P
  }
  
  // Check if we can see any enemies
  // TODO: how to detect team members?
  bool enemy = false;
  for (int i = 1; i < gs->maxPlayers; i++) {
    if (i != gs->playerEntityId && gs->entities[i].isVisible())
      enemy = true;
  }
  (*m_tempState)[ENEMY] = enemy ? VISIBLE : NOT_VISIBLE;
  
  int frags = gs->player.frags;
  if (frags < 0) {
    (*m_tempState)[FRAGS] = NEGATIVE_FRAGS;
  } else if (frags > MAX_FRAGS) {
    getLogger()->warning("Number of frags exceeded the server limit? Check the MAX_FRAG definition.");
    (*m_tempState)[FRAGS] = MAX_FRAGS;
  } else {
    (*m_tempState)[FRAGS] = frags;
  }
  
  return m_tempState;
}

void SoldierBrains::execute(BrainAction *action)
{
  // TODO Determine which physical state executes the needed action

  *m_currAction = *action;
  m_localPlanner->requestTransition(action->executionState()->getName());
}

}