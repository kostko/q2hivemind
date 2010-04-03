/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MAPPING_MAP_H
#define HM_MAPPING_MAP_H

#include "object.h"

#include <list>

namespace HiveMind {

class Context;
class MapPrivate;

/**
 * A complete Quake 2 BSP map implementation.
 */
class Map : public Object {
public:
    /**
     * Class constructor.
     *
     * @param context Hivemind context
     * @param name Map name
     */
    Map(Context *context, const std::string &name);
    
    /**
     * Class destructor.
     */
    virtual ~Map();
    
    /**
     * Opens the map, loading and linking it.
     */
    bool open();
protected:
    bool load();
    
    bool link();
private:
    // Context
    Context *m_context;
    
    // Paks
    std::list<std::string> m_paks;
    
    // Map metadata
    bool m_loaded;
    std::string m_name;
    
    // Map attributes
    MapPrivate *d;
};

}

#endif

