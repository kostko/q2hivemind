/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MOLD_MESSAGE_H
#define HM_MOLD_MESSAGE_H

#include <sstream>

// Protocol buffer generated classes
#include "src/mold/bus.pb.h"

namespace HiveMind {

namespace MOLD {

/**
 * Simple message parser.
 */
class Message {
public:
    // Header size, magic number and maximum body length
    enum { header_size = 8 };
    enum { magic = 0xDEADBEEF };
    enum { max_body_length = 102400 };
    
    /**
     * Class constructor.
     */
    Message();
    
    /**
     * Copy constructor.
     */
    Message(const Message &other);
    
    /**
     * Class destructor.
     */
    ~Message();
    
    /**
     * Returns the buffer used for this message.
     */
    inline const char *getData() const { return m_data; }
    
    /**
     * Returns the buffer used for this message.
     */
    inline char *getData() { return m_data; }
    
    /**
     * Returns the combined body and header lengths.
     */
    inline size_t getLength() const { return header_size + m_bodyLength; }
    
    /**
     * Returns the body length.
     */
    inline size_t getBodyLength() const { return m_bodyLength; }
    
    /**
     * Attempts to parse the header.
     *
     * @return False when body length is out of range
     */
    bool parseHeader();
    
    /**
     * Attempts to parse the body.
     *
     * @return False when body cannot be parsed
     */
    bool parseBody();
    
    /**
     * Encodes the packet.
     */
    void encode(const Protocol::Message &msg);
    
    /**
     * Returns the protocol message contained withing the packet.
     */
    inline Protocol::Message getProtocolMessage() const { return m_message; }
private:
    // Body length
    unsigned int m_bodyLength;
    
    // Data
    char *m_data;
    
    // Parsed message
    Protocol::Message m_message;
};

/**
 * Special-purpuse cast that extracts payload from an outer protocol message.
 */
template<typename T>
T message_cast(const Protocol::Message &msg)
{
  T payload;
  std::istringstream body(msg.data(), std::istringstream::in);
  payload.ParseFromIstream(&body);
  return payload;
}

}

}

#endif

