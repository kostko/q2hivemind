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
    case BotKilled: return "BotKilled";
    case BotLocationUpdate: return "BotLocationUpdate";
    case MapLinkUpdate: return "MapLinkUpdate";
    case LocationMetadataAdd: return "LocationMetadataAdd";
    case LocationMetadataClear: return "LocationMetadataClear";
    case EntityAppeared: return "EntityAppeared";
    case EntityDisappeared: return "EntityDisappeared";
    default: return "Invalid";
  }
}

BotKilledEvent::BotKilledEvent()
  : Event(BotKilled)
{
}

BotLocationUpdateEvent::BotLocationUpdateEvent(const Vector3f &origin)
  : Event(BotLocationUpdate),
    m_origin(origin)
{
}

MapLinkUpdateEvent::MapLinkUpdateEvent(int linkId)
  : Event(MapLinkUpdate),
    m_linkId(linkId),
    m_valid(true)
{
}

LocationMetadataAddEvent::LocationMetadataAddEvent(const Vector3f &location)
  : Event(LocationMetadataAdd)
{
}

LocationMetadataClearEvent::LocationMetadataClearEvent(const Vector3f &location)
  : Event(LocationMetadataClear)
{
}

EntityAppearedEvent::EntityAppearedEvent(int entityId, const Vector3f &origin)
  : Event(EntityAppeared)
{
}

EntityDisappearedEvent::EntityDisappearedEvent(int entityId)
  : Event(EntityDisappeared)
{
}


}


