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

#include <boost/unordered_set.hpp>

namespace HiveMind {

class Context;
class Directory;
class GlobalPlanner;

/**
 * Represents a bot entry in the directory.
 */
class Bot {
friend class Directory;
friend class GlobalPlanner;
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
     * Returns this bot's last known origin.
     */
    inline Vector3f getOrigin() const { return m_origin; }
    
    /**
     * Returns the age of this directory entry.
     */
    inline timestamp_t getAge() const { return Timing::getCurrentTimestamp() - m_lastUpdate; } 
protected:
    /**
     * Sets this bot's entity identifier.
     */
    inline void setEntityId(int entityId) { m_entityId = entityId; }
    
    /**
     * Sets the number of frags for this bot.
     */
    inline void setFragCount(int count) { m_fragCount = count; }
    
    /**
     * Sets this bot's last known origin.
     */
    inline void setOrigin(const Vector3f &origin) { m_origin = origin; }
    
    /**
     * Updates last update time for this entry.
     */
    inline void updateTime() { m_lastUpdate = Timing::getCurrentTimestamp(); }
private:
    // Bot name and attributes
    std::string m_name;
    int m_entityId;
    timestamp_t m_lastUpdate;
    
    // Exchanged metadata
    Vector3f m_origin;
    int m_fragCount;
};

// Set of bots
typedef boost::unordered_set<Bot*> BotSet;

/**
 * Bot team directory.
 */
class Directory : public Object {
friend class GlobalPlanner;
public:
    /**
     * Class constructor.
     *
     * @param context Bot context
     */
    Directory(Context *context);
    
    /**
     * Find a bot identified by its unique name.
     *
     * @param id Unique bot identifier
     * @return A valid Bot instance or NULL
     */
    Bot *getBotByName(const std::string &id) const;
    
    /**
     * Find a bot identified by its entity identifier.
     *
     * @param entityId Entity identifier
     * @return A valid Bot instance or NULL
     */
    Bot *getBotByEntityId(int entityId) const;
    
    /**
     * Returns true if the specified entity identifier represents
     * a friend (a bot registered with the system).
     */
    inline bool isFriend(int entityId) const { return getBotByEntityId(entityId) != NULL; }
    
    /**
     * Returns a set of currently registered bots.
     */
    BotSet getRegisteredBots() const;
protected:
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
     * Performs a garbage collection of bots that have not sent their
     * updates for more than 10 seconds.
     */
    void collect();
private:
    // Context
    Context *m_context;
    
    // Actual maps
    boost::unordered_map<std::string, Bot*> m_names;
    boost::unordered_map<int, Bot*> m_entities;
};

}

#endif

