/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "event.h"

namespace HiveMind {

Event::Event()
  : m_type(Invalid)
{
}

Event::Event(Type type)
  : m_type(type)
{
}

Event::~Event()
{
}

std::string Event::getTypeAsString() const
{
  switch (m_type) {
    case BotLocationUpdate: return "BotLocationUpdate";
    case BotRespawn: return "BotRespawn";
    case OpponentSpotted: return "OpponentSpotted";
    case EntityUpdated: return "EntityUpdated";
    case PollVoteCompleted: return "PollVoteCompleted";
    default: return "Invalid";
  }
}

BotEvent::BotEvent(Type type, Bot *bot)
  : Event(type),
    m_bot(bot)
{
}

BotLocationUpdateEvent::BotLocationUpdateEvent(Bot *bot, const Vector3f &origin)
  : BotEvent(BotLocationUpdate, bot),
    m_origin(origin)
{
}

BotRespawnEvent::BotRespawnEvent(Bot *bot)
  : BotEvent(BotRespawn, bot)
{
}

GoToAndDropWeaponEvent::GoToAndDropWeaponEvent(Bot *bot)
  : BotEvent(DropOrderIssued, bot)
{
}

OpponentSpottedEvent::OpponentSpottedEvent(const Vector3f& origin)
  : Event(OpponentSpotted),
    m_origin(origin)
{
}

EntityUpdatedEvent::EntityUpdatedEvent(const Entity &entity, bool external)
  : Event(EntityUpdated),
    m_entity(entity),
    m_external(external)
{
}

PollVoteCompletedEvent::PollVoteCompletedEvent(Poll *poll)
  : Event(PollVoteCompleted),
    m_poll(poll)
{
}

}


