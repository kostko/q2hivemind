/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mapping/dynamic.h"
#include "mapping/grid.h"
#include "mapping/items.h"
#include "context.h"
#include "logger.h"
#include "dispatcher.h"
#include "event.h"
#include "network/connection.h"
#include "planner/directory.h"
#include "planner/global.h"

namespace HiveMind {

DynamicMapper::DynamicMapper(Context *context)
  : m_context(context),
    m_grid(context->getGrid()),
    m_lastGridCollection(0),
    m_haveLastOrigin(false)
{
  Object::init();
  
  // Subscribe to update events
  Dispatcher *dispatcher = m_context->getDispatcher();
  dispatcher->signalBotLocationUpdate.connect(boost::bind(&DynamicMapper::botLocationUpdated, this, _1));
  dispatcher->signalBotRespawn.connect(boost::bind(&DynamicMapper::botRespawned, this, _1));
  dispatcher->signalEntityUpdated.connect(boost::bind(&DynamicMapper::entityUpdated, this, _1));
}

DynamicMapper::~DynamicMapper()
{
}

void DynamicMapper::botLocationUpdated(BotLocationUpdateEvent *event)
{
  // TODO incorporate team member movements to learn paths
  
  // TODO unordered map, last position for each path
  
  // TODO respawn event invalidates the previous location and does not
  //      learn a path (otherwise invalid links would be established)
}

void DynamicMapper::entityUpdated(EntityUpdatedEvent *event)
{
  processEntity(event->getEntity());
}

void DynamicMapper::processEntity(const Entity &entity)
{
  Connection *conn = m_context->getConnection();
  Directory *dir = m_context->getGlobalPlanner()->getDirectory();
  
  if (entity.isVisible() && entity.modelIndex > 0) {
    if (!entity.isPlayer()) {
      // Non player entity
      std::string model = conn->getServerConfig(entity.modelIndex);
      
      // We always learn its location regardless of type (note that this node
      // might be unlinked with the rest of the map and will get linked later
      // on as the dynamic mapper learns movements)
      GridNode *node = m_grid->getNodeByLocation(entity.origin);
      
      // Determine type and act accoordingly
      if (model.find("models/items") != std::string::npos || model.find("models/weapons") != std::string::npos) {
        // A valuable item
        Item item = Item::forModel(model);
        item.setLocation(entity.origin);
        item.updateLastSeen();
        node->setType(GridNode::Item);
        node->addItem(item);
        m_grid->learnItem(node);
      } else if (model.find("models/objects/dmspot/tris.md2") != std::string::npos) {
        // Spawn point
        node->setType(GridNode::SpawnPoint);
      } else {
        // Unknown non-player entity
      }
    } else {
      // Player entity
      if (!dir->isFriend(entity.getEntityId())) {
        if (m_lastEntityOrigin.find(entity.getEntityId()) != m_lastEntityOrigin.end()) {
          learnFromMovement(m_lastEntityOrigin.at(entity.getEntityId()), entity.serverOrigin);
        }
        
        m_lastEntityOrigin[entity.getEntityId()] = entity.serverOrigin;
      }
      
      // TODO Update composite dynamic entity view
    }
  } else if (entity.isPlayer() && !dir->isFriend(entity.getEntityId())) {
    // Invalidate last entity origin
    // FIXME maybe we should base this on entity seen timestamps
    m_lastEntityOrigin.erase(entity.getEntityId());
  }
}

void DynamicMapper::botRespawned(BotRespawnEvent *event)
{
  if (event->getBot() == NULL) {
    // Local bot, reset last origin so we don't learn this path
    m_haveLastOrigin = false;
  } else {
    // TODO non-local bot
  }
}

void DynamicMapper::learnFromMovement(const Vector3f &pointA, const Vector3f &pointB)
{
  if (pointA == pointB)
    return;
  
  // Check for spawn points
  float distance = (pointA - pointB).norm();
  GridNode *node = m_grid->getNodeByLocation(pointB);
  if (node->getType() == GridNode::SpawnPoint && distance > 50)
    return;
  
  // Sanity check if distance between points is too great, only learn individual
  // waypoints
  if (distance > 200) {
    m_grid->getNodeByLocation(pointA);
    m_grid->getNodeByLocation(pointB);
  } else {
    // Actually learn the path into the mapping grid
    m_grid->learnWaypoints(pointA, pointB);
  }
}

void DynamicMapper::worldUpdated(const GameState &state)
{
  // Track our movements and learn paths; we must use server origin without any
  // interpolation because otherwise we might learn invalid waypoints
  if (m_haveLastOrigin && m_lastOrigin != state.player.serverOrigin) {
    learnFromMovement(m_lastOrigin, state.player.serverOrigin);
  }
  
  // For each visited GridNode remember when we have last visited it
  GridNode *node = m_grid->getNodeByLocation(state.player.serverOrigin);
  node->updateLastVisit();
  
  m_lastOrigin = state.player.serverOrigin;
  m_haveLastOrigin = true;
  
  // Run grid expiry functions once in a while
  timestamp_t now = Timing::getCurrentTimestamp();
  if (now - m_lastGridCollection >= grid_collection_interval) {
    m_grid->collectAllExpired();
    m_lastGridCollection = now;  
  }
}

}


