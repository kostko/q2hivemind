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
#include "mold/client.h"

// Protocol buffer messages
#include "src/mold/control.pb.h"


namespace HiveMind {

DropWeaponState::DropWeaponState(Context *context)
  : WanderState(context, "dropweapon", 5000, true),
    m_i(0),
    m_dropLocation(Vector3f(0,0,0)),
    m_dropRequester(NULL)
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
  m_dropRequester = event->getBot();
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
    getContext()->getConnection()->writeConsoleAsync("drop " + currentWeapon);
    getContext()->getConnection()->writeConsoleAsync("drop " + weaponsAmmo[currentWeapon]);
  } else {
    std::string secondBestWeapon = getLocalPlanner()->bestWeaponInInventory(currentWeapon);

    if (secondBestWeapon != "Blaster" && m_gameState->inventory[secondBestWeapon] > 0) {
      getLogger()->info(format("2I can drop %s, as I have %d and %d ammo for it") % secondBestWeapon % m_gameState->inventory[secondBestWeapon] % m_gameState->inventory[weaponsAmmo[secondBestWeapon]]);
      getContext()->getConnection()->writeConsoleAsync("drop " + secondBestWeapon);
      getContext()->getConnection()->writeConsoleAsync("drop " + weaponsAmmo[secondBestWeapon]);
    }
  }
}

void DropWeaponState::goodbye()
{
  // Send a confirmation message to the winner
  MOLD::ClientPtr client = getContext()->getMOLDClient();
  client->deliver(MOLD::Protocol::Message::STOP_WAITING_FOR_DROP, m_dropRequester->getName());

  getLogger()->info("Sending MOLD message to drop requester that he should stop waiting!");
}

void DropWeaponState::processFrame()
{
  Map *map = getContext()->getMap();
  timestamp_t now = Timing::getCurrentTimestamp();
  Vector3f origin = m_gameState->player.origin;

  if ((m_dropLocation - m_gameState->player.origin).norm() < 50) {
    if (m_i % 100 == 0) {
      dropWeapon();
      getLocalPlanner()->requestTransition("wander");
    } else {
      getLogger()->info("AT DESTINATION TO DROP!");

      m_moveTarget = m_dropLocation;
      m_moveDestination = m_dropLocation;
      m_moveFire = false;
      m_moveJump = false;      
    }
    
    m_i++;

  } else {
    // By default we just run aside (even if there is no path found)
    m_moveTarget = m_moveDestination = Vector3f(1, 1, 0);
    m_moveFire = false;
    m_moveJump = false;

    // Check if we got stuck
    if (m_speed > 0) {
      int delta = now - m_lastFrameUpdate;
      if (delta > 0)
        m_speed = 0.05*(1000 * (origin - m_lastOrigin).norm()/delta + 19.0*m_speed);

      m_lastFrameUpdate += delta;
    } else {
      m_lastFrameUpdate = now;
      m_speed = 400.0;
      m_lastOrigin = origin;
    }

    // Follow current path
    if (m_hasNextPoint) {
      // Visit current location and check if we got to a point
      if (m_currentPath.visit(origin)) {
        Vector3f p = m_currentPath.getCurrent()->getLocation();
        //getLogger()->info(format("Got it. Next point is %f %f %f.") % p[0] % p[1] % p[2]);
        resetPointStatistics();
      }

      if (m_currentPath.isDestinationReached()) {
        // We have reached our destination
        getLogger()->info("Destination reached.");
        m_atDestination = true;

        // We want to compute new random path the next time we will go into this state (in processPlanning)
        m_hasNextPoint = false;
        return;
      } else {
        m_moveTarget = m_moveDestination = m_currentPath.getCurrent()->getLocation();
      }

      if (m_speed < 10) {
        // Request to recompute the path
        //getLogger()->warning("We are stuck, but should be following a path!");
        recomputePath();
      } else {
        // Check whether we will probably never reach our destination
        float distance = getDistanceToDestination();

        if (m_minDistance == -1 || distance < m_minDistance) {
          m_minDistance = distance;
          m_maxDistance = distance;
          m_lastMinChange = now;
          m_lastZ = origin[2];
        } else if (now - m_lastMinChange > 1000) {
          float diff = m_maxDistance - m_minDistance;
          float diffZ = m_lastZ - origin[2];
          //
          // Reasons for stalepoint:
          //
          //   * Fell somewhere and our path is actually invalid (check via Z difference)
          //     (we must recalculate the path)
          //   * We are circling a waypoint
          //     (we must treat the waypoint as visited)
          //   * We are stuck because a link is unwalkable
          //     (we must mark the link as invalid and recalculate the path)
          //
          //getLogger()->warning(format("Potential stalepoint detected at distance %f max diff %f Z diff %f!") % m_minDistance % diff % diffZ);

          if (diff > 25 && diffZ <= 24) {
            // Probably circling a waypoint
            //getLogger()->info("Probably circling a waypoint. Marking it as reached.");
            m_currentPath.skip();
            resetPointStatistics();
          } else if (diffZ > 24) {
            // Probably fell somewhere
            //getLogger()->info("Probably fell somewhere. Recomputing a new path.");
            recomputePath(true);
          } else {
            recomputePath(true);
          }

          return;
        }

        // Update max distance from last potential stalepoint when passed
        if (distance > m_maxDistance) {
          m_maxDistance = distance;
        }
      }
    }
  }



}

void DropWeaponState::processPlanning()
{
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


