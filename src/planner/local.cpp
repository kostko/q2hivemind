/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/local.h"
#include "logger.h"

#include <boost/foreach.hpp>

namespace HiveMind {

LocalPlanner::LocalPlanner(Context *context)
  : m_context(context),
    m_currentState(NULL),
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
}

void LocalPlanner::getBestMove(Vector3f *orientation, Vector3f *velocity, bool *fire) const
{
  *orientation = Vector3f(0, 0, 0);
  *velocity = Vector3f(0, 0, 0);
  *fire = false;
  
  if (m_currentState)
    m_currentState->getNextMove(orientation, velocity, fire);
}
    
void LocalPlanner::requestTransition(const TransitionRequest &request)
{
  if (m_states.find(request.state) == m_states.end())
    getLogger()->error(format("Attempted use of unregistered state '%s'!") % request.state);
  
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
  // TODO Decide if we need to change states
  
  // Perform current state frame processing
  if (m_currentState)
    m_currentState->processFrame(state);
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
      if (p.priority > bestPriority)
        rq = &p;
    }
    
    if (rq != NULL) {
      if (m_currentState)
        m_currentState->goodbye();
      
      m_currentState = m_states[rq->state];
      m_currentState->initialize(rq->metadata);
      
      // Remove request from the list
      m_transitionRequests.remove(*rq);
    }
    
    // Perform current state planning processing
    if (m_currentState)
      m_currentState->processPlanning();
    
    // Sleep some 200ms
    usleep(200000);
  }
}

}


