/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/local.h"
#include "mapping/map.h"
#include "context.h"
#include "logger.h"
#include "algebra.h"
#include "rl/action.h"
#include "rl/brainstate.h"
#include "brains/soldier.h"
#include "network/gamestate.h"
#include "network/connection.h"

#include <boost/foreach.hpp>
#include <limits>

namespace HiveMind {

LocalPlanner::LocalPlanner(Context *context)
  : m_context(context),
    m_currentState(NULL),
    m_worldUpdated(false),
    m_abort(false),
    m_brains(new SoldierBrains(this)),
    m_motionController(context),
    m_lastSave(0),
    m_lastWeapChange(0)
{
  Object::init();
}
    
LocalPlanner::~LocalPlanner()
{
}

void LocalPlanner::registerState(State *state)
{
  if (state == NULL)
    return;
  
  getLogger()->info(format("Registering new state '%s'...") % state->getName());
  m_states[state->getName()] = state;
}

void LocalPlanner::getBestMove(Vector3f *orientation, Vector3f *velocity, bool *fire)
{
  *orientation = Vector3f(0, 0, 0);
  *velocity = Vector3f(0, 0, 0);
  *fire = false;
  
  // If there is no current state we have nothing to do but stay idle
  if (!m_currentState)
    return;

  // Get destination and target coordinates from current state
  Map *map = m_context->getMap();
  Vector3f destination, target;
  bool jump;
  m_currentState->getNextTarget(&destination, &target, fire, &jump);
  
  // If there is no target we simply stay idle
  if (destination[0] == std::numeric_limits<float>::infinity())
    return;
  
  // Compute orientation and velocity vectors for given destination
  Vector3f delta = destination - m_gameState->player.origin;
  Vector3f eye = target - m_gameState->player.origin;

  float pitch = Algebra::pitchFromVect(eye);
  float yaw = Algebra::yawFromVect(eye);

  bool move = delta != Vector3f::Zero();

  // Use fuzzy logic only when we need to move
  if (move) {
    yaw = m_motionController.calculateMotion(*m_gameState, yaw, delta.norm());
  }

  (*velocity)[0] = move ? 400 : 0;
  (*velocity)[1] = 0;
  (*velocity)[2] = jump ? 400.0 : 0.0;

  (*orientation)[0] = -pitch;
  (*orientation)[1] = yaw;
  (*orientation)[2] = 0.0;
}

void LocalPlanner::requestTransition(const std::string &state, const boost::any &metadata)
{
  if (m_states.find(state) == m_states.end())
    getLogger()->error(format("Attempted use of unregistered state '%s'!") % state);

  if (m_currentState) {
    // Transition to state itself is not executed
    if (m_currentState->getName() == state)
      return;

    // Quit previous state
    m_currentState->goodbye();
    m_currentState->m_complete = true;
    getLogger()->info(format("Left '%s' state.") % m_currentState->getName());
  }

  // And enter new state
  m_currentState = m_states[state];
  m_currentState->initialize(metadata);
  getLogger()->info(format("Entered '%s' state.") % state);
}

void LocalPlanner::addEligibleState(State *state)
{  
  state->setEventStart(Timing::getCurrentTimestamp());
  m_eligibleStates.insert(state);
 // getLogger()->info(format("Adding %s to eligible states set.") % state->getName());
}

void LocalPlanner::pruneState(const std::string &state)
{
  if (m_states.find(state) == m_states.end())
    getLogger()->error(format("Attempted to prune unregistered state '%s'!") % state);

  State* stateObject = m_states[state];

  // Some states are not to be pruned
  if (!stateObject->isPrunable())
    return;

  // If the state is not in eligible states set, we can't prune it
  if (m_eligibleStates.find(stateObject) == m_eligibleStates.end())
    return;

  getLogger()->info(format("Pruning %s from eligible states set.") % state);

  m_eligibleStates.erase(stateObject);

  // If we are in the state that needs to be pruned, mark state as completed
  if (stateObject == m_currentState)
    m_currentState->m_complete = true;
}

bool LocalPlanner::canDropWeapon()
{
  std::string currentWeapon = m_gameState->player.getWeaponName();
  std::string secondBestWeapon = bestWeaponInInventory(currentWeapon);

  if (m_gameState->inventory.find(currentWeapon) == m_gameState->inventory.end())
    return false;

  if (m_gameState->inventory.find(secondBestWeapon) == m_gameState->inventory.end())
    return false;

  if (m_gameState->inventory[currentWeapon] > 1|| (secondBestWeapon != "Blaster" && m_gameState->inventory[secondBestWeapon] > 0))
    return true;

  return false;
}

void LocalPlanner::tryUseBetterWeapon()
{

  if (Timing::getCurrentTimestamp() - m_lastWeapChange < 2000 || m_gameState->player.health < 0)
    return;

  std::string currentWeapon = m_gameState->player.getWeaponName();
  std::string bestWeapon = bestWeaponInInventory();

  if (getAmmoForWeapon(currentWeapon) == 0) {
    currentWeapon = "Blaster";
  }

  boost::unordered_map<std::string, int> weapons;
  weapons["Grenades"] = 0;
  weapons["Blaster"] = 1;
  weapons["Shotgun"] = 2;
  weapons["Super Shotgun"] = 3;
  weapons["Machinegun"] = 4;
  weapons["Chaingun"] = 5;
  weapons["Grenade Launcher"] = 6;
  weapons["Rocket Launcher"] = 7;
  weapons["HyperBlaster"] = 8;
  weapons["Railgun"] = 9;
  weapons["BFG10K"] = 10;

  
  if (weapons.find(currentWeapon) == weapons.end() || weapons.find(bestWeapon) == weapons.end())
    return;

  // Change the weapon to better one
  if (weapons[currentWeapon] < weapons[bestWeapon]) {
    getContext()->getConnection()->use(bestWeapon);
    m_lastWeapChange = Timing::getCurrentTimestamp();
  }

}

int LocalPlanner::getAmmoForWeapon(const std::string &weapon)
{
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

  if (weaponsAmmo.find(weapon) == weaponsAmmo.end())
    return 0;

  std::string ammo = weaponsAmmo.at(weapon);

  if (m_gameState->inventory.find(ammo) == m_gameState->inventory.end())
    return 0;

  return m_gameState->inventory.at(ammo);
}

const std::string LocalPlanner::bestWeaponInInventory(const std::string &notWeapon)
{
  boost::unordered_map<std::string, int> weapons;

  weapons["Grenades"] = 0;
  weapons["Blaster"] = 1;
  weapons["Shotgun"] = 2;
  weapons["Super Shotgun"] = 3;
  weapons["Machinegun"] = 4;
  weapons["Chaingun"] = 5;
  weapons["Grenade Launchewer"] = 6;
  weapons["Rocket Launcher"] = 7;
  weapons["HyperBlaster"] = 8;
  weapons["Railgun"] = 9;
  weapons["BFG10K"] = 10;

  int maxPriority = 0;
  std::string currentWeapon = "Blaster";

  typedef std::pair<std::string, int> InventoryPair;
  BOOST_FOREACH(InventoryPair element, m_gameState->inventory) {
    std::string w = element.first;

    if (weapons.find(w) == weapons.end())
      continue;

    if (weapons[w] > maxPriority && getAmmoForWeapon(w) > 0 && w != notWeapon) {
      // There is a better weapon in our inventory and we have ammo for it

      // Special case can happen because of inventory not being up2date
      if (weapons.find(notWeapon) != weapons.end())
        if (weapons.at(notWeapon) <= weapons.at(w))
          continue;

      maxPriority = weapons.at(w);
      currentWeapon = w;
    }
  }

  return currentWeapon;
}

void LocalPlanner::updateEligibleStates()
{
  // Go through all states and check for event triggers
  // This will in turn populate our eligible states set and update its members' event start time
  typedef std::pair<std::string, State*> StatePair;
  BOOST_FOREACH(StatePair element, m_states) {
    State *state = element.second;

    if (m_gameState->player.health < 0 && state->getName() != "respawn")
      continue;

    state->checkEvent();
  }


  std::list<State*> pruneList;

  BOOST_FOREACH(State *state, m_eligibleStates) {
    int delta = Timing::getCurrentTimestamp() - state->getEventStart();

    // If state is too old add it to prune list
    if (delta > state->getEligibilityTime()) 
      pruneList.push_back(state);
    
  }

  // Prune all states that are too old
  BOOST_FOREACH(State *state, pruneList) {
    pruneState(state->getName());
  }

}

void LocalPlanner::start()
{
  // Init the brains
  m_brains->init();

  // Initialize the background worker thread
  m_workerThread = boost::thread(&LocalPlanner::process, this);
}

void LocalPlanner::worldUpdated(const GameState &state)
{
  // Update game state
  m_gameState = const_cast<GameState*>(&state);
  
  Map *map = m_context->getMap();
  Vector3f origin = state.player.origin;
  
  m_worldUpdated = true;

  typedef std::pair<std::string, State*> StatePair;
  BOOST_FOREACH(StatePair element, m_states) {
    State *state = element.second;
    state->m_gameState = m_gameState;
  }

  // Update eligible states set
  updateEligibleStates();

  tryUseBetterWeapon();

  // Let the brain process what to do
  m_brains->interact();
  
  // Perform current state frame processing
  if (m_currentState)
    m_currentState->processFrame();
}

void LocalPlanner::process()
{
  while (!m_abort) {
    // When there is no current state we transition to wander state
    if (!m_currentState)
      requestTransition("wander");
      
    // Perform current state planning processing
    if (m_currentState && m_worldUpdated)
      m_currentState->processPlanning();

    // Periodically save gained knowledge
    timestamp_t now = Timing::getCurrentTimestamp();
    if (now - m_lastSave > 30000 && m_brains->learnMode()) {
      m_brains->save();
      m_lastSave = now;
    }

    // Sleep some 200ms
    usleep(200000);
  }
}

void LocalPlanner::clearEligibleStates()
{
  // we delete all states except wander state
  BOOST_FOREACH(State *state, m_eligibleStates) {    
    if (state->isPrunable()) m_eligibleStates.erase(state);
  }
}
}


