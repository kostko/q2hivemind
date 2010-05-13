/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mold/message.h"

#include <sstream>
#include <arpa/inet.h>

namespace HiveMind {

namespace MOLD {

Message::Message()
  : m_bodyLength(0)
{
  m_data = new char[header_size];
}

Message::Message(const Message &other)
  : m_bodyLength(other.m_bodyLength),
    m_message(other.m_message)    
{
  m_data = new char[header_size + m_bodyLength];
  memcpy(m_data, other.m_data, header_size + m_bodyLength);
}

Message::~Message()
{
  delete m_data;
}

bool Message::parseHeader()
{
  // First check magic value
  unsigned int parsedMagic = ntohl(*((unsigned int*) m_data));
  if (parsedMagic != magic)
    return false;
  
  // Now parse body length and reallocate memory for accepting it
  m_bodyLength = ntohl(*((unsigned int*) (m_data + 4)));
  if (m_bodyLength > max_body_length)
    return false;
  
  m_data = static_cast<char*>(realloc(m_data, header_size + m_bodyLength));
  return true;
}

bool Message::parseBody()
{
  std::istringstream body(std::string(m_data + header_size, m_bodyLength), std::istringstream::in);
  return m_message.ParseFromIstream(&body);
}

void Message::encode(const Protocol::Message &msg)
{
  std::ostringstream body(std::ostringstream::out);
  msg.SerializeToOstream(&body);
  m_bodyLength = body.str().size();
  m_message = msg;
  
  // Prepare the message buffer
  m_data = new char[header_size + m_bodyLength];
  *((unsigned int*) m_data) = htonl(magic);
  *((unsigned int*) (m_data + 4)) = htonl(m_bodyLength);
  memcpy(m_data + 8, body.str().data(), m_bodyLength);
}

}

}


