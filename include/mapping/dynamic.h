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

namespace HiveMind {

class Context;
class Grid;

/**
 * A dynamic mapper learns from bot and other entity movements to
 * create a waypoint system via the mapping grid.
 */
class DynamicMapper : public Object {
public:
    DynamicMapper(Context *context);
    
    ~DynamicMapper();
    
    /**
     * Called on each frame after the world might have updated.
     *
     * @param state State of the world
     */
    void worldUpdated(const GameState &state);
private:
    // Context
    Context *m_context;
    
    // Mapping grid
    Grid *m_grid;
};

}

#endif

