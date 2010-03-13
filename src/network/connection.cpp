/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "network/connection.h"
#include "logger.h"
#include "timing.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

using boost::format;

namespace HiveMind {

Connection::Connection(const std::string &id, const std::string &host, int port)
  : m_host(host),
    m_port(boost::lexical_cast<std::string>(port)),
    m_connected(false),
    m_svSequence(0),
    m_clSequence(0),
    m_svBit(0),
    m_clBit(0),
    m_lastReliableSeq(0),
    m_lastPingTime(0),
    m_runningPing(0),
    m_challengeNum(0)
{
  Object::init();
  
  // Prepare configuration
  m_config["rate"] = "25000";
  m_config["msg"] = "1";
  m_config["fov"] = "90";
  m_config["skin"] = "male/flak";
  m_config["name"] = "hm_" + id;
  m_config["hand"] = "2";
  
  // Generate random client ID
  srand(Timing::getCurrentTimestamp());
  m_clientId = rand();
}
    
Connection::~Connection()
{
}
    
void Connection::connect()
{
  getLogger()->info(format("Attempting to connect with [%s]:%s...") % m_host % m_port);
  
  // Resolve hostname
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_protocol = 0;
  
  if (getaddrinfo(m_host.c_str(), m_port.c_str(), &hints, &result) != 0)
    getLogger()->error(format("Unable to resolve hostname '%s'!") % m_host);
  
  // Attempt to create a proper socket and bind it
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    m_socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (m_socket == -1)
      continue;
    
    if (::connect(m_socket, rp->ai_addr, rp->ai_addrlen) == 0)
      break;
  }
  
  if (rp == NULL)
    getLogger()->error(format("Unable to bind socket for host '[%s]:%s'!") % m_host % m_port);
  
  // Log partial success
  getLogger()->info(format("Host '%s' resolved, socket bound.") % m_host);
  
  // Create the processing thread
  m_workerThread = boost::thread(&Connection::worker, this);
  m_connected = false;
  
  // Attempt connection sequence
  getLogger()->info("Attempting to get challenge from server...");
  do {
    sendUnorderedPacket((char*) "getchallenge\x0a", 13);
    sleep(1);
  } while (!m_challengeNum);
  
  std::string options = "\x00";
  BOOST_FOREACH(StringPair p, m_config) {
    options += str(format("\\%s\\%s") % p.first % p.second);
  }
  
  getLogger()->info("Sending connect request...");
  do {
    char *buffer = (char*) str(format("connect 34 %d %d \"%s\"\x0a") % m_clientId % m_challengeNum % options).c_str();
    sendUnorderedPacket(buffer, strlen(buffer));
    sleep(1); 
  } while (!m_connected);
  
  // TODO console thread
  
  getLogger()->info("Connection established!");
}

void Connection::worker()
{
  char buffer[2048];
  int length;
  int result;
  
  do {
    length = receivePacket(buffer);
    if (length > 0) {
      // Process received packet
      result = processPacket(buffer, length);
    } else {
      result = 0;
    }
  } while (result == 0);
  
  m_connected = false;
  getLogger()->info("We are disconnected!");
}
    
int Connection::processPacket(char *buffer, size_t length)
{
  boost::lock_guard<boost::mutex> g(m_gameStateMutex);
  
  return 0;
}

void Connection::sendUnorderedPacket(char *data, size_t length)
{
  char buffer[2048];
  
  assert(length <= 2044);
  memcpy(buffer + 4, data, length);
  
  *((unsigned int*) buffer) = 0xffffffff;
  send(m_socket, buffer, length + 4, 0);
}

void Connection::sendUnreliablePacket(unsigned int seq, char *data, size_t length)
{
  boost::lock_guard<boost::mutex> g(m_sendURMutex);
  char buffer[2048];
  
  assert(length <= 2040);
  memcpy(buffer + 8, data, length);
  
  *((unsigned int*) buffer) = seq;
  *((unsigned int*) (buffer + 4)) = m_svSequence | m_svBit;
  send(m_socket, buffer, length + 8, 0); 
}
    
void Connection::sendReliablePacket(unsigned int seq, char *data, size_t length)
{
  boost::lock_guard<boost::mutex> g(m_sendRLMutex);
  char buffer[2048];
  
  assert(length <= 2040);
  memcpy(buffer + 8, data, length);
  
  *((unsigned int*) buffer) = seq | 0x80000000;
  *((unsigned int*) (buffer + 4)) = m_svSequence | m_svBit;
  
  // Send packet and wait for it to be ACKed
  m_reliableReceived = false;
  for (int i = 0; !m_reliableReceived; i++) {
    if (i % 10 == 0)
      send(m_socket, buffer, length + 8, 0);
    
    // Sleep for 50 msec
    usleep(50000);
  }
}

int Connection::receivePacket(char *data)
{
  char buffer[2048];
  char b[256];
  int length;
	unsigned int seq;
	unsigned int temp;
	int pingTime;
	
	// Emit status packets every 3 seconds
	pingTime = Timing::getCurrentTimestamp() - m_lastPingTime;
	if (pingTime > 3000) {
	  sprintf(b, "status");
	  sendUnorderedPacket(b, 7);
	  m_lastPingTime = Timing::getCurrentTimestamp();
	}
	
	// Receive a packet
	length = recv(m_socket, buffer, 2048, 0) - 8;
	if (length + 8 < 0)
	  getLogger()->error(format("Receive error %d (%s)!") % errno % strerror(errno));
	
	// XXX why is this not in packet processor ?
	seq = *((unsigned int *) buffer);
	if (seq == 0xffffffff) {
		buffer[length + 8] = 0;
		sscanf((char *)(buffer + 4), "%s %d", b, &temp);
		//getLogger()->info(format("packet %s") % b);
		
		if(!strcmp(b, "challenge")) {
		  // We got the challenge number
			m_challengeNum = temp;
		} else if(!strcmp(b, "print")) {
		  // We got RTT measurement packet
			pingTime = Timing::getCurrentTimestamp() - m_lastPingTime;
			m_runningPing = (2*m_runningPing + pingTime)/3;
		} else if(!strcmp(b, "client_connect")) {
		  // We got client connected packet
		  getLogger()->info("Received connect confirmation.");
			m_connected = true;
		}
		
		length = -1;
	} else {
		if (length)
		  memcpy(data, buffer + 8, length);
    
		m_svSequence = seq;
		if (m_svSequence & 0x80000000) {
		  // Server sent us a reliable packet
			m_svSequence &= 0x7fffffff;
			if (m_svSequence > m_lastReliableSeq) {
				m_lastReliableSeq = m_svSequence;
				m_svBit ^= 0x80000000;
			}
		}
		
		seq = *((unsigned int *) (buffer + 4));
		if ((seq & 0x80000000) != m_clBit) {
		  // Received reliable packet ACK
			m_clBit = seq & 0x80000000;
			m_reliableReceived = true;
		}
	}
	
	return length;
}

}

