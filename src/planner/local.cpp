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

#include <boost/foreach.hpp>

namespace HiveMind {

LocalPlanner::LocalPlanner(Context *context)
  : m_context(context),
    m_currentState(NULL),
    m_worldUpdated(false),
    m_abort(false)
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

void LocalPlanner::sideAdjust(Vector3f *delta) const
{
  Map *map = m_context->getMap();
  float t = sqrt((*delta)[0] * (*delta)[0] + (*delta)[1] * (*delta)[1]);
  if (t == 0)
    return;
  
  *delta = *delta / t;
  
  Vector3f a(0.0, 0.0, 0.0);
  Vector3f p = m_gameState.player.origin;
  float yaw = Algebra::yawFromVect(*delta);
  
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

void LocalPlanner::getBestMove(Vector3f *orientation, Vector3f *velocity, bool *fire) const
{
  *orientation = Vector3f(0, 0, 0);
  *velocity = Vector3f(0, 0, 0);
  *fire = false;
  
  // If there is no current state we have nothing to do but stay idle
  if (!m_currentState)
    return;

  // Get destination and target coordinates from current state
  Vector3f destination, target;
  m_currentState->getNextTarget(&destination, &target, fire);
  
  // Compute orientation and velocity vectors for given target
  Vector3f delta = target - m_gameState.player.origin;
  float pitch = Algebra::pitchFromVect(delta);
  float yaw = Algebra::yawFromVect(delta);
  
  delta = destination - m_gameState.player.origin;
  sideAdjust(&delta);
  
  float vx = delta[0] * (float) cos(-yaw) - delta[1] * (float) sin(-yaw);
  float vy = -delta[0] * (float) sin(-yaw) - delta[1] * (float) cos(-yaw);
  float vl = sqrt(vx*vx + vy*vy);
  
  (*orientation)[0] = pitch;
  (*orientation)[1] = yaw;
  (*orientation)[2] = 0.0;
  
  if (vl > 0) {
    (*velocity)[0] = 400.0 * vx/vl;
    (*velocity)[1] = 400.0 * vy/vl;
    (*velocity)[2] = 0.0; // No jumping:)
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

void LocalPlanner::start()
{
  // Initialize the background worker thread
  m_workerThread = boost::thread(&LocalPlanner::process, this);
}

void LocalPlanner::worldUpdated(const GameState &state)
{
  m_gameState = state;
  m_worldUpdated = true;
  
  // TODO Decide if we need to change states
  
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
    int bestPriority = m_currentState ? m_currentState->getPriority() : 0;
    TransitionRequest *rq = NULL;
    BOOST_FOREACH(TransitionRequest p, m_transitionRequests) {
      if (p.priority > bestPriority && (!m_currentState || p.state != m_currentState->getName()))
        rq = &p;
    }
    
    if (rq != NULL) {
      {
        boost::lock_guard<boost::mutex> g(m_stateMutex);
        if (m_currentState)
          m_currentState->goodbye();
        
        m_currentState = m_states[rq->state];
        m_currentState->initialize(rq->metadata);
      }
      
      // Remove requests from the list
      {
        boost::lock_guard<boost::mutex> g(m_requestMutex);
        m_transitionRequests.clear();
      }
    }
    
    // Perform current state planning processing
    if (m_currentState && m_worldUpdated)
      m_currentState->processPlanning();
    
    // Sleep some 200ms
    usleep(200000);
  }
}

}


