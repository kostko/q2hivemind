/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mold/server.h"
#include "logger.h"

namespace HiveMind {

namespace MOLD {

Connection::Connection(boost::asio::io_service &service, Server *server)
  : m_server(server),
    m_socket(service),
    m_clientId("")
{
  Object::init();
}
    
void Connection::start()
{
  // Read header
  boost::asio::async_read(
    m_socket,
    boost::asio::buffer(m_readMessage.getData(), Message::header_size),
    boost::bind(&Connection::handleReadHeader, shared_from_this(), boost::asio::placeholders::error)
  );
}

void Connection::deliver(const Message &msg)
{
  // Check that we did not send this message
  if (m_clientId != "" && m_clientId == msg.getProtocolMessage().sourceid())
    return;
  
  bool writeInProgress = !m_writeMessages.empty();
  m_writeMessages.push_back(msg);
  
  if (!writeInProgress) {
    boost::asio::async_write(
      m_socket,
      boost::asio::buffer(m_writeMessages.front().getData(), m_writeMessages.front().getLength()),
      boost::bind(&Connection::handleWrite, shared_from_this(), boost::asio::placeholders::error)
    );
  }
}

void Connection::handleReadHeader(const boost::system::error_code &error)
{
  if (!error && m_readMessage.parseHeader()) {
    // Read body
    boost::asio::async_read(
      m_socket,
      boost::asio::buffer(m_readMessage.getData() + Message::header_size, m_readMessage.getBodyLength()),
      boost::bind(&Connection::handleReadBody, shared_from_this(), boost::asio::placeholders::error)
    );
  } else {
    getLogger()->info("Failed to parse header or client disconnected.");
    m_server->remove(shared_from_this());
  }
}

void Connection::handleReadBody(const boost::system::error_code &error)
{
  if (!error) {
    // Parse body and deliver message
    if (m_readMessage.parseBody()) {
      m_clientId = m_readMessage.getProtocolMessage().sourceid();
      m_server->nameConnection(m_clientId, shared_from_this());
      m_server->deliver(m_readMessage);
    } else {
      // Failed to parse body, log this and ignore message
      getLogger()->warning("Unable to parse message body!");
    }
    
    // Read next message
    boost::asio::async_read(
      m_socket,
      boost::asio::buffer(m_readMessage.getData(), Message::header_size),
      boost::bind(&Connection::handleReadHeader, shared_from_this(), boost::asio::placeholders::error)
    );
  } else {
    getLogger()->info("Failed to parse body.");
    m_server->remove(shared_from_this());
  }
}

void Connection::handleWrite(const boost::system::error_code &error)
{
  // Write any additional messages that have been queued
  if (!error) {
    m_writeMessages.pop_front();
    if (!m_writeMessages.empty()) {
      boost::asio::async_write(
        m_socket,
        boost::asio::buffer(m_writeMessages.front().getData(), m_writeMessages.front().getLength()),
        boost::bind(&Connection::handleWrite, shared_from_this(), boost::asio::placeholders::error)
      );
    }
  } else {
    m_server->remove(shared_from_this());
  }
}

Server::Server(boost::asio::io_service &service, const tcp::endpoint &endpoint)
  : m_service(service),
    m_acceptor(service, endpoint)
{
  Object::init();
  
  // Prepare to accept next connection
  ConnectionPtr connection(new Connection(m_service, this));
  m_acceptor.async_accept(
    connection->getSocket(),
    boost::bind(&Server::handleAccept, this, connection, boost::asio::placeholders::error)
  );
  
  getLogger()->info("MOLD server initialized.");
}

void Server::handleAccept(ConnectionPtr connection, const boost::system::error_code &error)
{
  if (!error) {
    getLogger()->info("Received connection from MOLD client.");
    
    // Register and start this connection
    m_connections.insert(connection);
    connection->start();
    
    // Prepare next connection
    ConnectionPtr connection(new Connection(m_service, this));
    m_acceptor.async_accept(
      connection->getSocket(),
      boost::bind(&Server::handleAccept, this, connection, boost::asio::placeholders::error)
    );
  }
}

void Server::deliver(const Message &msg)
{
  if (msg.getProtocolMessage().has_destinationid()) {
    // Targeted message
    std::string id = msg.getProtocolMessage().destinationid();
    if (m_connectionMap.find(id) == m_connectionMap.end()) {
      getLogger()->warning("Silently dropping targeted message to unknown destination!");
      return;
    }
    
    ConnectionPtr conn = m_connectionMap.at(id);
    conn->deliver(msg);
  } else {
    // Broadcast message
    std::for_each(
      m_connections.begin(),
      m_connections.end(),
      boost::bind(&Connection::deliver, _1, boost::ref(msg))
    );
  }
}

void Server::remove(ConnectionPtr connection)
{
  m_connections.erase(connection);
  m_connectionMap.erase(connection->getClientId());
  getLogger()->info("MOLD client has disconnected.");
}

void Server::nameConnection(const std::string &name, ConnectionPtr connection)
{
  m_connectionMap[name] = connection;
}

}

}


