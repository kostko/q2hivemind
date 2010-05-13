/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_PLANNER_DIRECTORY_H
#define HM_PLANNER_DIRECTORY_H

#include "object.h"
#include "timing.h"

namespace HiveMind {

class Context;
class Directory;

/**
 * Represents a bot entry in the directory.
 */
class Bot {
friend class Directory;
public:
    /**
     * Class constructor.
     *
     * @param id Unique identifier
     * @param entityId Entity identifier
     */
    Bot(const std::string &id, int entityId);
    
    /**
     * Returns this bot's name (= unique identifier).
     */
    inline std::string getName() const { return m_name; }
    
    /**
     * Returns this bot's entity identifier.
     */
    inline int getEntityId() const { return m_entityId; }
    
    /**
     * Returns the number of frags achieved by this bot.
     */
    inline int getFragCount() const { return m_fragCount; }
    
    /**
     * Sets the number of frags for this bot.
     */
    inline void setFragCount(int count) { m_fragCount = count; }
    
    /**
     * Returns this bot's last known origin.
     */
    inline Vector3f getOrigin() const { return m_origin; }
    
    /**
     * Sets this bot's last known origin.
     */
    inline void setOrigin(const Vector3f &origin) { m_origin = origin; }
    
    /**
     * Updates last update time for this entry.
     */
    inline void updateTime() { m_lastUpdate = Timing::getCurrentTimestamp(); }
protected:
    /**
     * Sets this bot's entity identifier.
     */
    inline void setEntityId(int entityId) { m_entityId = entityId; }
private:
    // Bot name and attributes
    std::string m_name;
    int m_entityId;
    timestamp_t m_lastUpdate;
    
    // Exchanged metadata
    Vector3f m_origin;
    int m_fragCount;
};

/**
 * Bot team directory.
 */
class Directory : public Object {
public:
    /**
     * Class constructor.
     *
     * @param context Bot context
     */
    Directory(Context *context);
    
    /**
     * Registers a new bot with the directory.
     *
     * @param id Bot unique identifier (for MOLD and in-game nick)
     * @param entityId In-game player identifier
     * @return Created bot instance
     */
    Bot *registerBot(const std::string &id, int entityId);
    
    /**
     * Unregister an existing bot.
     *
     * @param id Bot unique identifier
     * @return Returns the unregistered bot (needs to be freed)
     */
    Bot *unregisterBot(const std::string &id);
    
    /**
     * Updates a bot's entity identifier.
     *
     * @param id Bot unique identifier
     * @param entityId Entity identifier
     */
    void updateEntityId(const std::string &id, int entityId);
    
    /**
     * Find a bot identified by its unique name.
     *
     * @param id Unique bot identifier
     * @return A valid Bot instance or NULL
     */
    Bot *getBotByName(const std::string &id);
    
    /**
     * Find a bot identified by its entity identifier.
     *
     * @param entityId Entity identifier
     * @return A valid Bot instance or NULL
     */
    Bot *getBotByEntityId(int entityId);
private:
    // Context
    Context *m_context;
    
    // Actual maps
    boost::unordered_map<std::string, Bot*> m_names;
    boost::unordered_map<int, Bot*> m_entities;
};

}

#endif

