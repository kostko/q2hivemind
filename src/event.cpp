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

}


