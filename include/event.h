/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_EVENT_H
#define HM_EVENT_H

#include "globals.h"

#include <string>

namespace HiveMind {

/**
 * Represents an event that is used for communication between bot
 * components.
 */
class Event {
public:
    /**
     * Possible event types.
     */
    enum Type {
      Invalid = 0,
      BotKilled,
      BotLocationUpdate,      
      OpponentSpotted,
      
      // This represents any event and should be the last one
      Any
    };
    
    /**
     * Constructs an invalid event
     */
    Event();
    
    /**
     * Constructs an event of the specified type.
     *
     * @param type Event type
     */
    Event(Type type);
    
    /**
     * Class destructor.
     */
    virtual ~Event();
    
    /**
     * Returns the type of this event.
     */
    inline Type getType() const { return m_type; }
    
    /**
     * Returns the type of this event in a string form.
     */
    std::string getTypeAsString() const;
    
    /**
     * Returns true if this event is a valid one.
     */
    inline bool isValid() const { return m_type != Invalid && m_type != Any; }
private:
    // Event type
    Type m_type;
};

class BotKilledEvent : public Event {
public:
    /**
     * Class constructor.
     */
    BotKilledEvent();
};

class BotLocationUpdateEvent : public Event {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    /**
     * Class constructor.
     *
     * @param origin Current bot origin
     */
    BotLocationUpdateEvent(const Vector3f &origin);
    
    /**
     * Returns the bot's location.
     */
    inline Vector3f getOrigin() const { return m_origin; }
private:
    // Current bot origin
    Vector3f m_origin;
};

class OpponentSpottedEvent : public Event {
public:
    /**
     * Class constructor.
     *
     * @param origin Current bot origin
     */
    OpponentSpottedEvent(const Vector3f &origin);

    /**
     * Returns the bot's location.
     */
    inline Vector3f getOrigin() const { return m_origin; }
private:
    // Current bot origin
    Vector3f m_origin;
};

}

#endif

