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

#include <boost/foreach.hpp>
#include <limits>

namespace HiveMind {

LocalPlanner::LocalPlanner(Context *context)
  : m_context(context),
    m_currentState(NULL),
    m_worldUpdated(false),
    m_abort(false),
    m_brains(new SoldierBrains(this)),
    m_motionController(context)
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
  
  // Setup pointers
  state->m_gameState = &m_gameState;
  state->m_lastGameState = &m_lastGameState;
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
  *fire = false;
  
  // If there is no target we simply stay idle
  if (destination[0] == std::numeric_limits<float>::infinity())
    return;
  
  // Compute orientation and velocity vectors for given destination
  Vector3f delta = destination - m_gameState.player.origin;
  float pitch = Algebra::pitchFromVect(delta);
  float yaw = Algebra::yawFromVect(delta);
  
  // Request the motion controller to calculate yaw
  yaw = m_motionController.calculateMotion(m_gameState, yaw);
  
  (*orientation)[0] = -pitch;
  (*orientation)[1] = yaw;
  (*orientation)[2] = 0.0;
  
  (*velocity)[0] = 400.0;
  (*velocity)[1] = 0;
  (*velocity)[2] = jump ? 400.0 : 0.0;
}

void LocalPlanner::requestTransition(const TransitionRequest &request)
{
  if (m_states.find(request.state) == m_states.end())
    getLogger()->error(format("Attempted use of unregistered state '%s'!") % request.state);
  
  boost::lock_guard<boost::mutex> g(m_requestMutex);
  m_transitionRequests.push_back(request);
}

void LocalPlanner::requestTransition(const std::string &state, int priority)
{
  TransitionRequest rq;
  rq.state = state;
  rq.priority = priority;
  rq.metadata = boost::any();
  requestTransition(rq);
}

void LocalPlanner::addEligibleState(State *state) {  
  state->setEventStart(Timing::getCurrentTimestamp());

  m_eligibleStates.insert(state);
  getLogger()->info(format("Adding %s to eligible states set.") % state->getName());
  
}

void LocalPlanner::pruneEligibleStates() {
  timestamp_t now = Timing::getCurrentTimestamp();

  std::list<State*> pruneList;

  BOOST_FOREACH(State *state, m_eligibleStates) {
    // some states are not to be pruned
    if (state->getEligibilityTime() == -1)
      continue;

    int delta = now - state->getEventStart();

    // prune state if it is too old
    if (delta > state->getEligibilityTime()) {
      getLogger()->info(format("Pruning %s from eligible states set.") % state->getName());

      pruneList.push_back(state);

      // if we are in the state that needs to be pruned, mark state as completed and call brains
      if (state == m_currentState) {
        m_currentState->m_complete = true;
        m_brains->interact();
      }
    }
  }

  BOOST_FOREACH(State *state, pruneList) {
    m_eligibleStates.erase(state);
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
  m_gameState = state;
  m_worldUpdated = true;
  
  Map *map = m_context->getMap();
  Vector3f origin = m_gameState.player.origin;
  
  // Go through all states and check for event triggers
  typedef std::pair<std::string, State*> StatePair;
  BOOST_FOREACH(StatePair element, m_states) {
    State *state = element.second;
    state->checkEvent();
  }

  pruneEligibleStates();
  
  // Perform current state frame processing
  if (m_currentState)
    m_currentState->processFrame();
  
  m_lastGameState = state;
}

void LocalPlanner::process()
{
  while (!m_abort) {
    // When there is no current state we transition to wander state
    if (!m_currentState) {
      requestTransition("wander");
    }
    
    // Process state transition requests
    {
      boost::lock_guard<boost::mutex> g(m_requestMutex);
      int bestPriority = 0;
      TransitionRequest rq;
      BOOST_FOREACH(TransitionRequest p, m_transitionRequests) {
        if (p.priority >= bestPriority && (!m_currentState || p.state != m_currentState->getName())) {
          rq = p;
          bestPriority = p.priority;
        }
      }
      
      if (rq.isValid()) {
        if (m_currentState) {
          // When current state has not yet finished, let's stack it
          if (!m_currentState->isComplete()) {
            getLogger()->info(format("State %s interrupted by %s state.")  % m_currentState->getName() % rq.state);
          } else {
            m_currentState->goodbye();
          }
        }
        
        m_currentState = m_states[rq.state];
        m_currentState->m_complete = false;
        
        // Log state switch
        getLogger()->info(format("Switching to %s state.") % m_currentState->getName());
        m_currentState->initialize(rq.metadata, rq.restored);
      }
      
      // Remove requests from the list
      m_transitionRequests.clear();
    }

    // Let the brain process what to do
    m_brains->interact();

    // Perform current state planning processing
    if (m_currentState && m_worldUpdated)
      m_currentState->processPlanning();

    // Sleep some 200ms
    usleep(200000);
  }
}

void LocalPlanner::clearEligibleStates()
{
  // we delete all states except wander state
  BOOST_FOREACH(State *state, m_eligibleStates) {
    if (state->getName() != "wander") m_eligibleStates.erase(state);
  }
}
}


