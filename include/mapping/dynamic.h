/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MAPPING_DYNAMIC_H
#define HM_MAPPING_DYNAMIC_H

#include "object.h"
#include "network/gamestate.h"
#include "timing.h"

namespace HiveMind {

class Context;
class Grid;
class BotLocationUpdateEvent;
class BotRespawnEvent;
class EntityUpdatedEvent;

/**
 * A dynamic mapper learns from bot and other entity movements to
 * create a waypoint system via the mapping grid.
 */
class DynamicMapper : public Object {
public:
    // Grid collection interval (in msec)
    enum { grid_collection_interval = 60000 };
    
    /**
     * Class constructor.
     *
     * @param context Hivemind context
     */
    DynamicMapper(Context *context);
    
    /**
     * Class destructor.
     */
    ~DynamicMapper();
    
    /**
     * Called on each frame after the world might have updated.
     *
     * @param state State of the world
     */
    void worldUpdated(const GameState &state);
protected:
    /**
     * This method gets called when a bot's location is updated.
     */
    void botLocationUpdated(BotLocationUpdateEvent *event);
    
    /**
     * This method gets called when a bot has respawned.
     */
    void botRespawned(BotRespawnEvent *event);
    
    /**
     * This method gets called when an entity has been updated.
     */
    void entityUpdated(EntityUpdatedEvent *event);
    
    /**
     * Processes an entity update.
     */
    void processEntity(const Entity &entity);
    
    /**
     * Learns path from movement between two points.
     *
     * @param pointA First location
     * @param pointB Second location
     */
    void learnFromMovement(const Vector3f &pointA, const Vector3f &pointB);
private:
    /**
     * Check for new eligible states.
     */
    void checkEligible(const std::string &model);

    // Context
    Context *m_context;
    
    // Mapping grid
    Grid *m_grid;
    timestamp_t m_lastGridCollection;
    
    // Last position
    bool m_haveLastOrigin;
    Vector3f m_lastOrigin;
    
    // For tracking other entities
    boost::unordered_map<int, Vector3f> m_lastEntityOrigin;
};

}

#endif

