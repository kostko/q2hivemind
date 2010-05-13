/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MOLD_CLIENT_H
#define HM_MOLD_CLIENT_H

#include "object.h"
#include "mold/message.h"

#include <deque>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>

using boost::asio::ip::tcp;

namespace HiveMind {

class Context;

namespace MOLD {

/**
 * MOLD message bus client.
 */
class Client : public Object {
public:
    /**
     * Class constructor.
     *
     * @param service IO service used for async operations
     * @param endpoints TCP destinations to try
     */
    Client(Context *context, boost::asio::io_service &service, tcp::resolver::iterator endpoints);
    
    /**
     * Returns true if the connection is established.
     */
    inline bool isConnected() const { return m_connected; }
    
    /**
     * Dispatches a message to all bus participants.
     *
     * @param msg Message to deliver
     */
    void deliver(const Message &msg);
    
    /**
     * Dispatches a message to all bus participants.
     *
     * @param type Message type
     * @param msg Message to deliver
     */
    void deliver(int type, google::protobuf::Message *msg, const std::string &destinationId = "");
    
    /**
     * Closes connection with the bus.
     */
    void disconnect();
public:
    // Signals
    boost::signals2::signal<void ()> signalConnected;
    boost::signals2::signal<void (const Protocol::Message &msg)> signalMessageReceived;
protected:
    /**
     * Handler for establishing a connection
     *
     * @param error Potential error code
     * @param endpoints Untried endpoints
     */
    void handleConnect(const boost::system::error_code &error, tcp::resolver::iterator endpoints);
    
    /**
     * Handler for reading a header.
     *
     * @param error Potential error code
     */
    void handleReadHeader(const boost::system::error_code &error);
    
    /**
     * Handler for reading a body.
     *
     * @param error Potential error code
     */
    void handleReadBody(const boost::system::error_code &error);
    
    /**
     * Writes a message to the bus.
     *
     * @param message Message to write
     */
    void write(Message message);
    
    /**
     * Handler for writing a message.
     *
     * @param error Potential error code
     */
    void handleWrite(const boost::system::error_code &error);
    
    /**
     * Closes the connection.
     */
    void close();
private:
    // Context
    Context *m_context;
    
    // ASIO service and TCP socket
    boost::asio::io_service &m_service;
    tcp::socket m_socket;
    
    // Message currently being read and a queue for outgoing messages
    Message m_readMessage;
    std::deque<Message> m_writeMessages;
    
    // Connection status
    bool m_connected;
};

// Shared pointer for client class
typedef boost::shared_ptr<Client> ClientPtr;

}

}

#endif

