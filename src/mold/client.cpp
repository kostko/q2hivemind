/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mold/client.h" 
#include "logger.h"
#include "context.h"

#include <sstream>

namespace HiveMind {

namespace MOLD {

Client::Client(Context *context, boost::asio::io_service &service, tcp::resolver::iterator endpoints)
  : m_context(context),
    m_service(service),
    m_socket(service),
    m_connected(false)
{
  Object::init();
  
  // Attempt connections one by one
  tcp::endpoint endpoint = *endpoints;
  m_socket.async_connect(
    endpoint,
    boost::bind(&Client::handleConnect, this, boost::asio::placeholders::error, ++endpoints)
  ); 
}

void Client::deliver(int type, google::protobuf::Message *msg)
{
  Protocol::Message packet;
  packet.set_sourceid(m_context->getBotId());
  packet.set_timestamp(static_cast<unsigned int>(std::time(0)));
  packet.set_type(static_cast<Protocol::Message::PacketType>(type));
  std::ostringstream body(std::ostringstream::out);
  msg->SerializeToOstream(&body);
  packet.set_data(body.str());
  
  Message rawMsg;
  rawMsg.encode(packet);
  deliver(rawMsg);
}

void Client::deliver(const Message &msg)
{
  m_service.post(boost::bind(&Client::write, this, msg));
}

void Client::disconnect()
{
  m_service.post(boost::bind(&Client::close, this));
}

void Client::handleConnect(const boost::system::error_code &error, tcp::resolver::iterator endpoints)
{
  if (!error) {
    getLogger()->info("Successfully connected with MOLD message bus!");
    m_connected = true;
    signalConnected();
    
    // Connection established, get ready to read some packets
    boost::asio::async_read(
      m_socket,
      boost::asio::buffer(m_readMessage.getData(), Message::header_size),
      boost::bind(&Client::handleReadHeader, this, boost::asio::placeholders::error)
    );
  } else if (endpoints != tcp::resolver::iterator()) {
    // Try another endpoint
    tcp::endpoint endpoint = *endpoints;
    m_socket.close();
    m_socket.async_connect(
      endpoint,
      boost::bind(&Client::handleConnect, this, boost::asio::placeholders::error, ++endpoints)
    ); 
  } else {
    // Failed to establish connection
    getLogger()->info("Failed to establish connection with MOLD message bus, we are screwed!");
  }
}

void Client::handleReadHeader(const boost::system::error_code &error)
{
  if (!error && m_readMessage.parseHeader()) {
    // Read body
    boost::asio::async_read(
      m_socket,
      boost::asio::buffer(m_readMessage.getData() + Message::header_size, m_readMessage.getBodyLength()),
      boost::bind(&Client::handleReadBody, this, boost::asio::placeholders::error)
    );
  } else {
    getLogger()->warning("Failed to read message header, closing connection!");
    close();
  }
}

void Client::handleReadBody(const boost::system::error_code &error)
{
  if (!error) {
    if (!m_readMessage.parseBody()) {
      getLogger()->warning("Failed to parse message body!");
    } else {
      // We have received a message
      signalMessageReceived(m_readMessage.getProtocolMessage());
    }
    
    // Read next message
    boost::asio::async_read(
      m_socket,
      boost::asio::buffer(m_readMessage.getData(), Message::header_size),
      boost::bind(&Client::handleReadHeader, this, boost::asio::placeholders::error)
    );
  }
}

void Client::write(Message message)
{
  bool writeInProgress = !m_writeMessages.empty();
  m_writeMessages.push_back(message);
  
  if (!writeInProgress) {
    boost::asio::async_write(
      m_socket,
      boost::asio::buffer(m_writeMessages.front().getData(), m_writeMessages.front().getLength()),
      boost::bind(&Client::handleWrite, this, boost::asio::placeholders::error)
    );
  }
}

void Client::handleWrite(const boost::system::error_code &error)
{
  // Write any additional messages that have been queued
  if (!error) {
    m_writeMessages.pop_front();
    if (!m_writeMessages.empty()) {
      boost::asio::async_write(
        m_socket,
        boost::asio::buffer(m_writeMessages.front().getData(), m_writeMessages.front().getLength()),
        boost::bind(&Client::handleWrite, this, boost::asio::placeholders::error)
      );
    }
  } else {
    getLogger()->warning("An error ocurred while writing, closing connection!");
    close();
  }
}

void Client::close()
{
  getLogger()->warning("Disconnecting from MOLD message bus, we are screwed!");
  m_socket.close();
}

}

}


