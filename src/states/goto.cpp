/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "states/goto.h"
#include "planner/local.h"
#include "logger.h"
#include "context.h"

#include <boost/foreach.hpp>

namespace HiveMind {

bool item_cmp(ItemValue a, ItemValue b)
{
  return a.second > b.second;
}

GoToState::GoToState(Context *context, const std::string &name)
  : WanderState(context, name, 60000, true),
    m_lastTime(0),
    m_exists(false)
{
}

void GoToState::initialize(const boost::any &metadata)
{
  // Remember last state entry
  m_lastTime = Timing::getCurrentTimestamp();
  m_atDestination = false;
  m_complete = false;

  m_hasNextPoint = false;
  m_speed = 0;
  m_minDistance = -1;
  m_randomize = false;
}

void GoToState::goodbye()
{
}

void GoToState::checkEvent()
{
  // Check if enough time has passed
  timestamp_t now = Timing::getCurrentTimestamp();
  if ((now - m_lastTime > BETWEEN_GOTO) && itemExists()) {
    makeEligible();
  }
}

void GoToState::processPlanning()
{
  // We have (supposedly) picked up an item
  if (m_atDestination) {
    m_complete = true;
    getLocalPlanner()->pruneState(getName());
    return;
  }

  Vector3f p = m_gameState->player.origin;
  Grid *grid = getContext()->getGrid();
  GridWaypoint target(p);

  // Plan a path if none is currently available
  if (!m_hasNextPoint) {
    getLogger()->info(format("[%s] Planning a path to a useful item.") % getName());

    // Re-check our needs
    evaluate();

    // The state will be complete if we don't find a suitable item.
    m_complete = true;

    // Loop from the most needed to the least needed item
    BOOST_FOREACH(ItemValue t, m_items) {
      // Find the nearest node that contains our item
      GridNode *node = grid->getNearestItemNode(t.first, p);
      if (node == NULL) {
        continue;
      }

      // Find its exact location
      Vector3f loc = Vector3f::Zero();
      BOOST_FOREACH(Item item, node->items()) {
        if (item.getType() == t.first) {
          loc = item.getLocation();
        }
      }

      if (grid->findPath(p, loc, &m_currentPath)) {
        getLogger()->info(format("Discovered a path of length %d to an item.") % m_currentPath.size());
        m_hasNextPoint = true;
        m_complete = false;
        break;
      } else {
        getLogger()->info("Path to item not found.");
      }
    }

    // No item was available
    if (m_complete) {
      setItemExists(false);
    }

    if (!m_hasNextPoint) {
      m_complete = true;
      getLocalPlanner()->pruneState(getName());
    }
  }
}

void GoToState::makeEligible()
{
  getLocalPlanner()->addEligibleState(this);
}

}