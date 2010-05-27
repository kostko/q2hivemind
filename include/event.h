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
#include "network/gamestate.h"

#include <string>

namespace HiveMind {

class Bot;
class Poll;

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
      BotLocationUpdate,
      BotRespawn,
      EntityUpdated,
      OpponentSpotted,
      PollVoteCompleted,
      
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

/**
 * Superclass for events that are bot-dependent.
 */
class BotEvent : public Event {
public:
    /**
     * Class constructor.
     *
     * @param type Actual event type
     * @param bot Bot directory entry or NULL for local bot
     */
    BotEvent(Type type, Bot *bot);
    
    /**
     * Returns the bot's directory entry.
     */
    inline Bot *getBot() const { return m_bot; }
private:
    // Bot instance
    Bot *m_bot;
}; 

class BotLocationUpdateEvent : public BotEvent {
public:
    /**
     * Class constructor.
     *
     * @param bot Bot directory entry or NULL for local bot
     * @param origin Current bot origin
     */
    BotLocationUpdateEvent(Bot *bot, const Vector3f &origin);
    
    /**
     * Returns the bot's location.
     */
    inline Vector3f getOrigin() const { return m_origin; }
private:
    // Bot origin
    Vector3f m_origin;
};

class BotRespawnEvent : public BotEvent {
public:
    /**
     * Class constructor.
     *
     * @param bot Bot directory entry or NULL for local bot
     */
    BotRespawnEvent(Bot *bot);
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

class EntityUpdatedEvent : public Event {
public:
    /**
     * Class constructor.
     *
     * @param entity Entity that has been updated
     * @param external Is this an external update (via MOLD)
     */
    EntityUpdatedEvent(const Entity &entity, bool external = false);

    /**
     * Returns the entity.
     */
    inline Entity getEntity() const { return m_entity; }
    
    /**
     * Returns true if this entity update is an external one.
     */
    inline bool isExternal() const { return m_external; }
private:
    // Current entity origin
    Entity m_entity;
    bool m_external;
};

class PollVoteCompletedEvent : public Event {
public:
    /**
     * Class constructor.
     *
     * @param poll Poll that has been completed
     */
    PollVoteCompletedEvent(Poll *poll);
    
    /**
     * Returns the poll that has just been completed. Do not save
     * this pointer as the poll will be deleted right after this
     * event is delivered to all subscribers!
     */
    inline Poll *getPoll() const { return m_poll; }
private:
    // Completed poll
    Poll *m_poll;
};

}

#endif

