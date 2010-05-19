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

namespace HiveMind {

LocalPlanner::LocalPlanner(Context *context)
  : m_context(context),
    m_currentState(NULL),
    m_worldUpdated(false),
    m_abort(false)
{
  Object::init();

  // Setup brains
  m_brains = new SoldierBrains(this);
  
  // Setup sensors
  m_sensors[0] = DistanceSensor(context);
  m_sensors[0].setAngle(45);
  m_sensors[1] = DistanceSensor(context);
  m_sensors[1].setAngle(0);
  m_sensors[2] = DistanceSensor(context);
  m_sensors[2].setAngle(-45);
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

void LocalPlanner::sideAdjust(Vector3f *delta) const
{
  Map *map = m_context->getMap();
  float yaw = Algebra::yawFromVect(*delta);
  float t = sqrt((*delta)[0] * (*delta)[0] + (*delta)[1] * (*delta)[1]);
  if (t == 0)
    return;
  
  *delta = *delta / t;
  
  Vector3f a(0.0, 0.0, 0.0);
  Vector3f p = m_gameState.player.origin;
  
  for (int i = -8; i <= 8; i++) {
    float angle = yaw + (float) i * (M_PI/32.0);
    Vector3f b(
      m_gameState.player.origin[0] + 64.0 * cos(angle),
      m_gameState.player.origin[1] + 64.0 * sin(angle),
      0.0
    );
    t = 0;
    
    for (float height = -16; height < 32; height += 16) {
      p[2] = m_gameState.player.origin[2] + height;
      b[2] = p[2];
      float s = 1.0 - map->rayTest(p, b, Map::Solid);
      if (s > t)
        t = s;
    }
    
    a[0] -= t * cos(angle);
    a[1] -= t * sin(angle);
  }
  
  Vector3f b(
    cos(yaw + (M_PI/2.0)),
    sin(yaw + (M_PI/2.0)),
    0.0
  );
  t = a.dot(b);
  *delta += t*b;
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
  Vector3f destination, target;
  bool jump;
  m_currentState->getNextTarget(&destination, &target, fire, &jump);
  
  // Compute orientation and velocity vectors for given target
  Vector3f delta = target - m_gameState.player.origin;
  float pitch = Algebra::pitchFromVect(delta);
  float yaw = Algebra::yawFromVect(delta);
  
  // Update all sensors
  for (int i = 0; i < 3; i++) {
    m_sensors[i].update(m_gameState, yaw);
  }
  
  delta = destination - m_gameState.player.origin;
  sideAdjust(&delta);
  
  float vx = delta[0] * (float) cos(-yaw) - delta[1] * (float) sin(-yaw);
  float vy = -delta[0] * (float) sin(-yaw) - delta[1] * (float) cos(-yaw);
  float vl = sqrt(vx*vx + vy*vy);
  
  (*orientation)[0] = -pitch;
  (*orientation)[1] = yaw;
  (*orientation)[2] = 0.0;
  
  if (vl > 0) {
    (*velocity)[0] = 400.0 * vx/vl;
    (*velocity)[1] = 400.0 * vy/vl;
    (*velocity)[2] = jump ? 400.0 : 0.0;
  }
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
  timestamp_t now = Timing::getCurrentTimestamp();
  
  if (m_eligibleStates.find(state) == m_eligibleStates.end()) {
    m_eligibleStates.insert(state);
    getLogger()->info(format("Adding %s to eligible states set.") % state->getName());
  }
}

void LocalPlanner::pruneEligibleStates() {
  timestamp_t now = Timing::getCurrentTimestamp();

  std::set<State*>::iterator i = m_eligibleStates.begin();
  while (i != m_eligibleStates.end())
  {

    // some states are not to be pruned
    if ((*i)->getEligibilityTime() == -1) {
      ++i;
      continue;
    }

    int delta = now - (*i)->getEventStart();

    // prune state if it is too old 
    if (delta > (*i)->getEligibilityTime()) {
      getLogger()->info(format("Pruning %s from eligible states set.") % (*i)->getName());
      
      m_eligibleStates.erase(i);
    }

    ++i;

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
  
  // Go through all states and check if any would like to interrupt
  typedef std::pair<std::string, State*> StatePair;
  BOOST_FOREACH(StatePair element, m_states) {
    State *state = element.second;
    if (state != m_currentState)
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
      int bestPriority = m_currentState ? m_currentState->getPriority() : 0;
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

}


