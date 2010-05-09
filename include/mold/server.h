/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MOLD_SERVER_H
#define HM_MOLD_SERVER_H

#include "object.h"
#include "mold/message.h"

#include <deque>
#include <set>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

namespace HiveMind {

namespace MOLD {

class Server;

/**
 * Represents a connection to the MOLD server.
 */
class Connection : public Object, public boost::enable_shared_from_this<Connection> {
public:
    /**
     * Class constructor.
     *
     * @param service IO service used for async operations
     */
    Connection(boost::asio::io_service &service, Server *server);
    
    /**
     * Handle new connection establishment.
     */
    void start();
    
    /**
     * Queues a message for delivery.
     *
     * @param msg Message to deliver
     */
    void deliver(const Message &msg);
    
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
     * Handler for writing a message.
     *
     * @param error Potential error code
     */
    void handleWrite(const boost::system::error_code &error);
    
    /**
     * Returns the TCP socket used by this connection.
     */
    inline tcp::socket &getSocket() { return m_socket; }
private:
    // Server
    Server *m_server;
    
    // TCP socket
    tcp::socket m_socket;
    
    // Message currently being read and a queue for outgoing messages
    Message m_readMessage;
    std::deque<Message> m_writeMessages;
    
    // Known client identifier
    std::string m_clientId;
};

// Shared pointer for connection class
typedef boost::shared_ptr<Connection> ConnectionPtr;

/**
 * MOLD message bus server.
 */
class Server : public Object {
public:
    /**
     * Class constructor.
     *
     * @param service IO service used for async operations
     * @param endpoint TCP endpoint to listen on
     */
    Server(boost::asio::io_service &service, const tcp::endpoint &endpoint);
    
    /**
     * Dispatches a message to all bus participants.
     *
     * @param msg Message to deliver
     */
    void deliver(const Message &msg);

    /**
     * Removes the specified connection from the list of active
     * connections.
     *
     * @param connection Connection instance
     */    
    void remove(ConnectionPtr connection);
protected:
    /**
     * Connection accept handler.
     *
     * @param connection Connection to use
     * @param error Potential error code
     */
    void handleAccept(ConnectionPtr connection, const boost::system::error_code &error);
private:
    // ASIO service and TCP acceptor
    boost::asio::io_service &m_service;
    tcp::acceptor m_acceptor;
    
    // Established connections
    std::set<ConnectionPtr> m_connections;
};

// Shared pointer for server class
typedef boost::shared_ptr<Server> ServerPtr;

}

}

#endif

