/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include <boost/foreach.hpp>

#include "states/dropweapon.h"
#include "planner/local.h"
#include "logger.h"
#include "context.h"
#include "dispatcher.h"
#include "network/connection.h"
#include "planner/directory.h"
#include "rl/brains.h"
#include "mapping/grid.h"

namespace HiveMind {

DropWeaponState::DropWeaponState(Context *context)
  : WanderState(context, "dropweapon", 5000, true),
    m_lastDropTime(0),
    m_dropLocation(Vector3f(0,0,0))
{
  Object::init();

  getContext()->getDispatcher()->signalDropOrder.connect(boost::bind(&DropWeaponState::dropOrderReceived, this, _1));
}

DropWeaponState::~DropWeaponState()
{
}

void DropWeaponState::dropOrderReceived(GoToAndDropWeaponEvent *event)
{
  getLogger()->info(format("I HAVE RECEIVED A DROP ORDER AND WILL GO TO DROP NOW!!!! %f %f %f") % event->getBot()->getOrigin().x() % event->getBot()->getOrigin().y() % event->getBot()->getOrigin().z());
  getLogger()->info(format("I AM NOW AT %f %f %f") % m_gameState->player.origin.x() % m_gameState->player.origin.y() % m_gameState->player.origin.z());

  m_dropLocation = event->getBot()->getOrigin();
  
  getLocalPlanner()->requestTransition("dropweapon");  

}

void DropWeaponState::initialize(const boost::any &metadata)
{
  m_complete = false;
  m_hasNextPoint = false;
  m_atDestination = false;
}

void DropWeaponState::dropWeapon()
{
  std::string currentWeapon = m_gameState->player.getWeaponName();
  //getContext()->getConnection()->refreshInventory();

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


  if (m_gameState->inventory.find(currentWeapon) == m_gameState->inventory.end())
    return;

  if (m_gameState->inventory[currentWeapon] > 1 && getLocalPlanner()->getAmmoForWeapon(currentWeapon) > 0) {
    getLogger()->info(format("I can drop %s, as I have %d") % currentWeapon % m_gameState->inventory[currentWeapon]);
    //getContext()->getConnection()->say("Dropping "+currentWeapon + " and " + weaponsAmmo[currentWeapon]);
    getContext()->getConnection()->writeConsoleAsync("drop " + currentWeapon);
    getContext()->getConnection()->writeConsoleAsync("drop " + weaponsAmmo[currentWeapon]);
  } else {
    std::string secondBestWeapon = getLocalPlanner()->bestWeaponInInventory(currentWeapon);

    if (secondBestWeapon != "Blaster" && m_gameState->inventory[secondBestWeapon] > 0) {
      getLogger()->info(format("2I can drop %s, as I have %d and %d ammo for it") % secondBestWeapon % m_gameState->inventory[secondBestWeapon] % m_gameState->inventory[weaponsAmmo[secondBestWeapon]]);
      getContext()->getConnection()->writeConsoleAsync("drop " + secondBestWeapon);
      getContext()->getConnection()->writeConsoleAsync("drop " + weaponsAmmo[secondBestWeapon]);
      //getContext()->getConnection()->say("Dropping "+secondBestWeapon + " and " + weaponsAmmo[secondBestWeapon]);
    }
  }
}

void DropWeaponState::goodbye()
{
}

void DropWeaponState::processPlanning()
{
  if ((m_dropLocation - m_gameState->player.origin).norm() < 50) {
    getLogger()->info("AT DESTINATION TO DROP!");
    dropWeapon();
    getLocalPlanner()->requestTransition("wander");
  } else {
    Vector3f p = m_gameState->player.origin;
    Grid *grid = getContext()->getGrid();

    // Plan a path if none is currently available
    if (!m_hasNextPoint) {
      getLogger()->info(format("PLANNING A PATH TO %f %f %f") % m_dropLocation.x() % m_dropLocation.y() % m_dropLocation.z());
      if (grid->findPath(p, m_dropLocation, &m_currentPath)) {
        getLogger()->info(format("Discovered a path of length %d.") % m_currentPath.size());
        m_hasNextPoint = true;
        resetPointStatistics();
      } else {
        getLogger()->info("Sorry, not coming to drop as I didn't find a path.");
        m_complete = true;
      }
    }

  }
}

}


