/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "logger.h"
#include "planner/local.h"
#include "planner/global.h"
#include "planner/directory.h"
#include "brains/soldier.h"
#include "context.h"
 
namespace HiveMind {

SoldierBrains::SoldierBrains(HiveMind::LocalPlanner *planner)
  : Brains(planner)
{
}

SoldierBrains::~SoldierBrains()
{
}

void SoldierBrains::init()
{
  std::vector<int> stateComps;
  stateComps.push_back(N_STATE0);
  stateComps.push_back(N_STATE1);
  stateComps.push_back(N_STATE2);
  stateComps.push_back(N_STATE3);
  stateComps.push_back(N_STATE4);

  std::vector<int> actionComps;
  actionComps.push_back(N_ACTIONS);

  Brains::init(stateComps, actionComps);

  // Init the weapon to ID map
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

  // Init the action ID to State map
  m_actionStateMap[WANDER] = m_localPlanner->getStateFromName("wander");
  m_actionStateMap[SHOOT_AT] = m_localPlanner->getStateFromName("shoot");
  m_actionStateMap[FIND_AMMO] = m_localPlanner->getStateFromName("gotoammo");
  m_actionStateMap[FIND_HEALTH] = m_localPlanner->getStateFromName("gotohealth");
  m_actionStateMap[FIND_BETTER_WEAPON] = m_localPlanner->getStateFromName("gotoweapon");
  m_actionStateMap[FIND_UPGRADE] = m_localPlanner->getStateFromName("gotoupgrade");

  // Initial execution state
  m_currAction->setExecutionState(m_actionStateMap[WANDER]);
  m_currAction->setName("wander");

  // Load the data according to the bot's id
  std::string id = m_localPlanner->getContext()->getBotId();
  m_Q->load("data/" + id + ".knowledge");
  m_numQ->load("data/" + id + ".visits");

  getLogger()->info("Brains initialized.");
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
  GameState *gs = m_localPlanner->getGameState();

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
  bool enemy = false;
  for (int i = 1; i < gs->maxPlayers; i++) {

    // Check for friendly fire :P
    bool isFriend = m_localPlanner->getContext()->getGlobalPlanner()->getDirectory()->isFriend(i);

    if (i != gs->playerEntityId && gs->entities[i].isVisible() && !isFriend) {
      enemy = true;
    }
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
  // Get the state associated with the action
  State *executionState = m_actionStateMap[action->id()];

  // Prepare the action to execute
  *m_currAction = *action;
  m_currAction->setExecutionState(executionState);
  m_currAction->setName(executionState->getName());
  
  // Request the transition
  m_localPlanner->requestTransition(executionState->getName());
}

bool SoldierBrains::eligibleAction(BrainAction *action)
{
  State *ex = m_actionStateMap[action->id()];
  return m_localPlanner->isEligible(ex);
}

std::string SoldierBrains::defaultActionName()
{
  return m_actionStateMap[alwaysEligibleId()]->getName();
}

void SoldierBrains::save()
{
  std::string id = m_localPlanner->getContext()->getBotId();
  m_Q->save("data/" + id + ".knowledge");
  m_numQ->save("data/" + id + ".visits");
}

}
