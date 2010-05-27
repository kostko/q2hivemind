/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/dropweapon.h"
#include "planner/local.h"
#include "logger.h"
#include "context.h"
#include "dispatcher.h"
#include "network/connection.h"
#include "planner/directory.h"

namespace HiveMind {

DropWeaponState::DropWeaponState(Context *context)
  : State(context, "dropweapon", 5000, true),
    m_lastDropTime(0)
{
  Object::init();
  getContext()->getDispatcher()->signalBotRespawn.connect(boost::bind(&DropWeaponState::botRespawned, this, _1));
}

DropWeaponState::~DropWeaponState()
{
}

void DropWeaponState::botRespawned(BotRespawnEvent *event)
{
  if (event->getBot() == NULL) {
    // We have respawned
  } else {
    // Some other bot has respawned
    // TODO:
    // 1) Check if I am near enough
    // 2) Check if I have anything to drop
    // 3) Say via MOLD that I am coming to drop a weapon
    // 4) If bot has confirmed, then go there
    // 5) Drop a weapon
  }
}

void DropWeaponState::initialize(const boost::any &metadata)
{
  m_complete = false;

  std::string currentWeapon = m_gameState->player.getWeaponName();
  boost::unordered_map<std::string, std::string> weaponsAmmo;
  weaponsAmmo["Blaster"] = "Blaster";
  weaponsAmmo["Shotgun"] = "Shells";
  weaponsAmmo["Super Shotgun"] = "Shells";
  weaponsAmmo["Machinegun"] = "Bullets";
  weaponsAmmo["Chaingun"] = "Bullets";
  weaponsAmmo["Grenade Launcher"] = "Grenades";
  weaponsAmmo["Rocket Launcher"] = "Rockets";
  weaponsAmmo["HyperBlaster"] = "Cells";
  weaponsAmmo["Railgun"] = "Slugs";
  weaponsAmmo["BFG10K"] = "Cells";

  getContext()->getConnection()->refreshInventory();

  if (m_gameState->inventory.find(currentWeapon) == m_gameState->inventory.end())
    return;

  if (weaponsAmmo.find(currentWeapon) == weaponsAmmo.end())
    return;

  if (m_gameState->inventory[currentWeapon] > 1) {
    getLogger()->info(format("I can drop %s, as I have %d") % currentWeapon % m_gameState->inventory[currentWeapon]);
    getContext()->getConnection()->say("Dropping "+currentWeapon + " and " + weaponsAmmo[currentWeapon]);
    getContext()->getConnection()->writeConsoleAsync("drop " + currentWeapon);
    getContext()->getConnection()->writeConsoleAsync("drop " + weaponsAmmo[currentWeapon]);
  } else {
    std::string secondBestWeapon = getLocalPlanner()->bestWeaponInInventory(currentWeapon);

    if (m_gameState->inventory.find(secondBestWeapon) == m_gameState->inventory.end())
      return;

    if (weaponsAmmo.find(secondBestWeapon) == weaponsAmmo.end())
      return;

    if (secondBestWeapon != "Blaster" && m_gameState->inventory[secondBestWeapon] > 0) {
      getLogger()->info(format("2I can drop %s, as I have %d and %d ammo for it") % secondBestWeapon % m_gameState->inventory[secondBestWeapon] % m_gameState->inventory[weaponsAmmo[secondBestWeapon]]);
      getContext()->getConnection()->writeConsoleAsync("drop " + secondBestWeapon);
      getContext()->getConnection()->writeConsoleAsync("drop " + weaponsAmmo[secondBestWeapon]);
      getContext()->getConnection()->say("Dropping "+secondBestWeapon + " and " + weaponsAmmo[secondBestWeapon]);
    }
  }

  getContext()->getConnection()->refreshInventory();
}

void DropWeaponState::goodbye()
{
}

void DropWeaponState::processFrame()
{
  Vector3f dest = Vector3f(0.0, 0.0, 0.0);
  m_moveTarget = m_moveDestination = dest;
  m_moveFire = false;
  m_moveJump = false;
  
  getLocalPlanner()->requestTransition("wander");
}

void DropWeaponState::checkEvent()
{
  std::string currentWeapon = m_gameState->player.getWeaponName();
  std::string secondBestWeapon = getLocalPlanner()->bestWeaponInInventory(currentWeapon);

  if (m_gameState->inventory.find(currentWeapon) == m_gameState->inventory.end())
    return;

  if (m_gameState->inventory.find(secondBestWeapon) == m_gameState->inventory.end())
    return;

  if (Timing::getCurrentTimestamp() - m_lastDropTime > 11000 &&
      (m_gameState->inventory[currentWeapon] > 1 || (secondBestWeapon != "Blaster" && m_gameState->inventory[secondBestWeapon] > 0))) {
    getLocalPlanner()->requestTransition("dropweapon");
    m_lastDropTime = Timing::getCurrentTimestamp();    
  }
}

}


