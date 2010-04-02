/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "network/connection.h"
#include "network/util.h"
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
    m_online(false),
    m_svSequence(0),
    m_clSequence(0),
    m_svBit(0),
    m_clBit(0),
    m_lastReliableSeq(0),
    m_lastPingTime(0),
    m_runningPing(0),
    m_challengeNum(0),
    m_currentState(0),
    m_currentFrame(0),
    m_lastFrame(0xffffffff),
    m_deltaFrame(0xffffffff),
    m_cs(&(m_gamestates[0])),
    m_ds(&(m_gamestates[16])),
    m_spawn(&(m_gamestates[0])),
    m_lastInventoryUpdate(0),
    m_currentUpdate(0),
    m_lastUpdateTime(0)
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
  m_workerThread = boost::thread(&Connection::workerProtocol, this);
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
  
  // Spawn a console thread
  m_consoleThread = boost::thread(&Connection::workerConsole, this);
  getLogger()->info("Connection established!");
  
  // Precache and come online
  getLogger()->info("Precaching...");
  writeConsoleSync("new");
  {
    boost::unique_lock<boost::mutex> lock(m_onlineMutex);
    while (!m_online) {
      m_onlineCond.wait(lock);
    }
  }
  
  getLogger()->info("We are synced to server!");
}

void Connection::begin()
{
  writeConsoleSync(str(format("begin %d\n") % m_loginKey));
  getLogger()->info("Enter the cage!");
}

void Connection::disconnect()
{
  if (m_online) {
    m_online = false;
    writeConsoleSync("disconnect");
    
    // Terminate threads
    m_consoleThread.interrupt();
    m_workerThread.interrupt();
  }
  
  getLogger()->info("Client disconnected.");
}

void Connection::say(const std::string &msg)
{
  writeConsoleSync("say " + msg);
}

void Connection::move(const Vector3f &angles, const Vector3f &velocity, bool attack)
{
  boost::lock_guard<boost::mutex> g(m_gameStateMutex);
  Vector3f adjAngles;
  int frameTime;
  
  if (!m_online)
    return;
  
  frameTime = Timing::getCurrentTimestamp() - m_lastUpdateTime;
  while (frameTime < 10) {
    usleep(10000);
    frameTime = Timing::getCurrentTimestamp() - m_lastUpdateTime;
  }
  m_lastUpdateTime += frameTime;
  if (frameTime > 255)
    frameTime = 255;
  
  adjAngles[0] = angles[0] + m_cs->player.angles[0];
  adjAngles[1] = angles[1] - m_cs->player.angles[1];
  adjAngles[2] = angles[2];
  
  m_updates[m_currentUpdate].angles = adjAngles;
  m_updates[m_currentUpdate].velocity = velocity;
  m_updates[m_currentUpdate].msec = frameTime;
  m_updates[m_currentUpdate].light = 0;
  m_updates[m_currentUpdate].buttons = attack ? 0x81 : 0;
  m_updates[m_currentUpdate].impulse = 0;
  m_updates[m_currentUpdate].timestamp = Timing::getCurrentTimestamp();
  
  dispatchUpdate();
}

void Connection::dispatchUpdate()
{
  char buffer[2048];
  unsigned char mask;
  unsigned int seq;
  int i, n, m;
  
  seq = ++m_clSequence;
  i = 0;
  
  *((unsigned short*) (buffer + i)) = m_clientId;
  i += 2;
  
  buffer[i++] = 0x02;
  buffer[i++] = 0x00;
  *((unsigned long*) (buffer + i)) = m_currentFrame | m_packetLoss;
  i += 4;
  
  for (int j = -3; j < 0; j++) {
    n = (m_currentUpdate + MAX_UPDATES + j + 1) % MAX_UPDATES;
    m = (m_currentUpdate + MAX_UPDATES + j) % MAX_UPDATES;
    if (j == -3)
      m = MAX_UPDATES;
    
    mask = 0;
    if (m_updates[n].angles[0] != m_updates[m].angles[0]) mask |= 0x01; // pitch
    if (m_updates[n].angles[1] != m_updates[m].angles[1]) mask |= 0x02; // yaw
    if (m_updates[n].angles[2] != m_updates[m].angles[2]) mask |= 0x04; // roll
    if (m_updates[n].velocity[0] != m_updates[m].velocity[0]) mask |= 0x08;
    if (m_updates[n].velocity[1] != m_updates[m].velocity[1]) mask |= 0x10;
    if (m_updates[n].velocity[2] != m_updates[m].velocity[2]) mask |= 0x20;
    if (m_updates[n].buttons != m_updates[m].buttons) mask |= 0x40;
    if (m_updates[n].impulse != m_updates[m].impulse) mask |= 0x80;
    buffer[i++] = mask;
    
    if (mask & 0x01) {
      *((short*) (buffer + i)) = (short) (-m_updates[n].angles[0] * 32768.0/M_PI);
      i += 2;
    }
    if (mask&0x02) {
      *((short*) (buffer + i)) = (short) (m_updates[n].angles[1] * 32768.0/M_PI);
      i += 2;
    }
    if (mask & 0x04) {
      *((short*) (buffer + i)) = (short) (m_updates[n].angles[2] * 32768.0/M_PI);
      i += 2;
    }
    if(mask & 0x08) {
      *((short*) (buffer + i)) = (short) (m_updates[n].velocity[0]);
      i += 2;
    }
    if(mask & 0x10) {
      *((short*) (buffer + i)) = (short) (m_updates[n].velocity[1]);
      i += 2;
    }
    if (mask & 0x20) {
      *((short*) (buffer + i)) = (short) (m_updates[n].velocity[2]);
      i += 2;
    }
    if(mask & 0x40)
      buffer[i++] = m_updates[n].buttons;
    if(mask & 0x80)
      buffer[i++] = m_updates[n].impulse;
    
    buffer[i++] = m_updates[n].msec;
    buffer[i++] = m_updates[n].light;
  }
  
  m_currentUpdate = (m_currentUpdate + 1) % MAX_UPDATES;
  buffer[3] = Util::checksum((unsigned char*) (buffer + 4), i - 4, seq);
  sendUnreliablePacket(seq, buffer, i); 
}

void Connection::workerConsole()
{
  getLogger()->info("Console thread is up and running.");
  
  try {
    for (;;) {
      boost::unique_lock<boost::mutex> lock(m_consoleMutex);
      while (m_consoleQueue.empty()) {
        m_consoleCond.wait(lock);
      }
      
      // Process queued data
      writeConsoleSync(m_consoleQueue.front());
      m_consoleQueue.pop_front();
    }
  } catch (boost::thread_interrupted e) {
    getLogger()->info("Console thread is terminating.");
  }
}

void Connection::writeConsoleSync(const std::string &msg)
{
  int length;
  char buffer[2048];
  
  // Check if we are connected
  if (!m_connected) {
    getLogger()->warning("Attempted a console write while not connected!");
    return;
  }
  
  length = msg.length() + 1;
  assert(length <= 2045);
  
  *((unsigned short *) buffer) = m_clientId;
  buffer[2] = 0x04;
  memcpy(buffer + 3, msg.c_str(), length);
  sendReliablePacket(++m_clSequence, buffer, length + 3);
}

void Connection::writeConsoleAsync(const std::string &msg)
{
  // Atomically insert an item into the queue
  {
    boost::lock_guard<boost::mutex> lock(m_consoleMutex);
    m_consoleQueue.push_back(msg);
  }
  
  // Notify the console thread that the queue now contains a task
  m_consoleCond.notify_all();
}

void Connection::workerProtocol()
{
  char buffer[2048];
  int length;
  int result;
  
  getLogger()->info("Protocol thread is up and running.");
  
  do {
    length = receivePacket(buffer);
    if (length > 0) {
      // Process received packet
      result = processPacket(buffer, length);
      m_cs->timestamp = Timing::getCurrentTimestamp();
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
  
  // Parse incoming packet
  int i = 0;
  unsigned char type;
  char s[256];
  int i_start = 0;
  int timestamp = Timing::getCurrentTimestamp();
  
  while (i < length) {
    type = buffer[i++];
    
    switch (type) {
      // Muzzleflash
      case 0x01: i += 3; break;
      
      // Monster Muzzleflash
      case 0x02: i += 3; break;
      
      // Temporary entity
      case 0x03: {
        unsigned char entityType = buffer[i++];
        
        switch(entityType) {
          case 5:
          case 6:
          case 7:
          case 8:
          case 17:
          case 18:
          case 20:
          case 21:
          case 22:
          case 28: {
            i += 6;
            break;
          }

          case 0:
          case 1:
          case 2:
          case 4:
          case 9:
          case 12:
          case 13:
          case 14:
          case 27: {
            i += 7;
            break;
          }

          case 3:
          case 11:
          case 23:
          case 26: {
            i += 12;
            break;
          }

          case 10: {
            i += 9;
            break;
          }

          case 15:
          case 25:
          case 29: {
            i += 9;
            break;
          }

          case 16:
          case 19: {
            i += 14;
            break;
          }

          case 24: {
            i += 20;
            break;
          }

          default: {
            getLogger()->warning(format("Unrecognized entity type %d!") % entityType);
            return 0;
          }
        }
        break;
      }
      
      // Layout
      case 0x04: {
        while (buffer[i++]);
        break;
      }
      
      // Inventory
      case 0x05: {
        for (int j = 0; j < 256; j++) {
          m_inventory[j] = *((short *) (buffer + i));
          i += 2;
        }
        m_lastInventoryUpdate = Timing::getCurrentTimestamp();
        break;
      }
      
      // NOOP
      case 0x06: break;
      
      // Disconnect
      case 0x07: return 1;
      
      // Reconnect
      case 0x08: return 1;
      
      // Sound
      case 0x09: {
        unsigned int mask = (unsigned char) buffer[i++];
        i++;

        if (mask & 0x01) i++;
        if (mask & 0x02) i++;
        if (mask & 0x10) i++;
        if (mask & 0x08) i += 2;
        if (mask & 0x04) i += 6;
        break;
      }
      
      // Print
      case 0x0a: {
        int j = 0;
        i++;

        while (buffer[i++]) {
          s[j++] = buffer[i-1] & 0x7f;
        }

        s[j++] = 0;
        getLogger()->info(format("SERVER PRINT: %s") % s);
        break;
      }
      
      // StuffText (client transfers commands from the server and executes
      // them in its console)
      case 0x0b: {
        int j = 0;
        while (buffer[i++]) {
          s[j++] = buffer[i - 1];
        }
        s[j++] = 0;
        
        // Check what we got
        std::string command(s);
        if (command.find("precache") != std::string::npos) {
          getLogger()->info("Precache completed.");
          
          // Atomically update our online status
          {
            boost::lock_guard<boost::mutex> lock(m_onlineMutex);
            m_online = true;
          }
          
          // Notify the console thread that the queue now contains a task
          m_onlineCond.notify_all();
        } else if (command.find("cmd") != std::string::npos) {
          // Execute the command that the server wants us to execute
          writeConsoleAsync(command.substr(4));
        } else {
          // Unknown StuffText
          getLogger()->warning(format("Received unknown StuffText command: %s") % command);
        }
        break;
      }
      
      // Serverinfo
      case 0x0c: {
        m_serverVersion = *((int*) (buffer + i));
        i += 4;
        m_loginKey = *((int*) (buffer + i));
        i += 4;
        buffer[i++] = 1;
        while (buffer[i++]);
        m_playerNum = *((short*) (buffer + i)) + 1;
        i += 2;
        while (buffer[i++]);
        
        // Show some information
        getLogger()->info(format("Server protocol version: %d") % m_serverVersion);
        getLogger()->info(format("Server login key: %d") % m_loginKey);
        getLogger()->info(format("Player number: %d") % m_playerNum);
        break;
      }
      
      // ConfigString
      case 0x0d: {
        int num;
        int j = 0;
        
        num = *((short *) (buffer + i));
        i += 2;
        
        while (buffer[i]) {
          m_serverConfig[num] += buffer[i++];
        }
        i++;
        //getLogger()->info(format("Server config string[%d] = '%s' (%d)") % num % m_serverConfig[num] % m_serverConfig[num].length());
        
        // Handle max number of players config string
        if (num == 30) {
          m_maxPlayers = boost::lexical_cast<int>(m_serverConfig[num]);
        }
        break;
      }
      
      // SpawnEntity
      case 0x0e: {
        unsigned int mask;
        int entity;

#define READ_CHAR ((unsigned char) buffer[i++])

        mask = READ_CHAR;
        if (mask & 0x00000080) mask |= (READ_CHAR << 8);
        if (mask & 0x00008000) mask |= (READ_CHAR << 16);
        if (mask & 0x00800000) mask |= (READ_CHAR << 24);
        if (mask & 0x00000100) {
          entity = *((short*) (buffer + i));
          i += 2;
        } else {
          entity = READ_CHAR;
        }

        // Sanity check for entity identifier
        if (entity >= 1024) {
          getLogger()->error("Entity number greater than 1024! Protocol violation, aborting.");
        }

        m_spawn->entities[entity].modelIndex = (mask & 0x00000800) ? READ_CHAR : 0;
        m_spawn->entities[entity].modelIndex2 = (mask & 0x00100000) ? READ_CHAR : 0;
        m_spawn->entities[entity].modelIndex3 = (mask & 0x00200000) ? READ_CHAR : 0;
        m_spawn->entities[entity].modelIndex4 = (mask & 0x00400000) ? READ_CHAR : 0;
        m_spawn->entities[entity].framenum = 0;

        if (mask & 0x00000010)
          m_spawn->entities[entity].framenum = READ_CHAR;
        if (mask & 0x00020000) {
          m_spawn->entities[entity].framenum = *((short*) (buffer + i));
          i += 2;
        }
        if (mask & 0x00010000) {
          if (mask & 0x02000000) {
            i += 4;
          } else {
            i++;
          }
        } else {
          if (mask & 0x02000000) {
            i += 2;
          }
        }
        if (mask & 0x00004000) {
          if (mask & 0x00080000) {
            i += 4;
          } else {
            i++;
          }
        } else {
          if (mask & 0x00080000)
            i+=2;
        }
        if (mask & 0x00001000) {
          if (mask & 0x00040000) {
            m_spawn->entities[entity].renderfx = *((int*) (buffer + i));
            i += 4;
          } else {
            m_spawn->entities[entity].renderfx = READ_CHAR;
          }
        } else {
          if (mask & 0x00040000) {
            m_spawn->entities[entity].renderfx = *((short*) (buffer + i));
            i += 2;
          } else {
            m_spawn->entities[entity].renderfx = 0;
          }
        }
        if (mask & 0x00000001) {
          m_spawn->entities[entity].origin[0] = 0.125 * ((float) *((short*) (buffer + i)));
          i += 2;
        } else {
          m_spawn->entities[entity].origin[0] = 0;
        }
        if (mask & 0x00000002) {
          m_spawn->entities[entity].origin[1] = 0.125 * ((float) *((short*) (buffer + i)));
          i += 2;
        } else {
          m_spawn->entities[entity].origin[1] = 0;
        }
        if (mask & 0x00000200) {
          m_spawn->entities[entity].origin[2] = 0.125 * ((float) *((short*) (buffer + i)));
          i += 2;
        } else {
          m_spawn->entities[entity].origin[2] = 0;
        }

        m_spawn->entities[entity].angles[0] = (mask & 0x00000400) ? (M_PI/128.0 * (float) buffer[i++]) : 0;
        m_spawn->entities[entity].angles[1] = (mask & 0x00000004) ? (M_PI/128.0 * (float) buffer[i++]) : 0;
        m_spawn->entities[entity].angles[2] = (mask & 0x00000008) ? (M_PI/128.0 * (float) buffer[i++]) : 0;
        if (mask & 0x01000000) i += 6;
        if (mask & 0x04000000) i++;
        if (mask & 0x00000020) i++;
        if (mask & 0x08000000) i += 2;

        // Mark entity as not updated (not visible)
        m_spawn->entities[entity].setVisible(false);
        m_dataPoints[entity].timestamp = timestamp;
        m_dataPoints[entity].origin[0] = m_spawn->entities[entity].origin[0];
        m_dataPoints[entity].origin[1] = m_spawn->entities[entity].origin[1];
        m_dataPoints[entity].origin[2] = m_spawn->entities[entity].origin[2];
        break;
      }
      
      // CenterPrint
      case 0x0f: {
        while(buffer[i++]);
        break;
      }
      
      // Download
      case 0x10: {
        int size;
        int percent;
        
        size = *((unsigned short*) (buffer + i));
        i += 2;
        percent = buffer[i++];
        if (size > -1) {
          i += size;
        }
        break;
      }
      
      // Playerinfo
      case 0x11: {
        unsigned int mask;
        mask = *((unsigned short*) (buffer + i));
        i += 2;
        
        if (mask & 0x0001)
          i++;
        if (mask & 0x0002) {
          // Origin update
          m_cs->player.origin[0] = 0.125 * ((float) *((short*) (buffer + i)));
          i += 2;
          m_cs->player.origin[1] = 0.125 * ((float) *((short*) (buffer + i)));
          i += 2;
          m_cs->player.origin[2] = 0.125 * ((float) *((short*) (buffer + i)));
          i += 2;
        }
        if (mask & 0x0004) {
          // Velocity update
          m_cs->player.velocity[0] = 0.0125 * ((float) *((short*) (buffer + i)));
          i += 2;
          m_cs->player.velocity[1] = 0.0125 * ((float) *((short*) (buffer + i)));
          i += 2;
          m_cs->player.velocity[2] = 0.0125 * ((float) *((short*) (buffer + i)));
          i += 2;
        }
        if (mask & 0x0008) i++;
        if (mask & 0x0010) i++;
        if (mask & 0x0020) i += 2;
        if (mask & 0x0040) {
          // Orientation update
          m_cs->player.angles[0] = M_PI/32768.0 * ((float) *((short*) (buffer + i)));
          i += 2;
          m_cs->player.angles[1] = M_PI/32768.0 * ((float) *((short*) (buffer + i)));
          i += 2;
          m_cs->player.angles[2] = M_PI/32768.0 * ((float) *((short*) (buffer + i)));
          i += 2;
        }
        if (mask & 0x0080) i += 3;
        if (mask & 0x0100) i += 6;
        if (mask & 0x0200) i += 3;
        if (mask & 0x1000) m_cs->player.gunindex = buffer[i++];
        if (mask & 0x2000) i += 7;
        if (mask & 0x0400) i += 4;
        if (mask & 0x0800) i++;
        if (mask & 0x4000) i++;
        
        // Update player stats
        mask = *((unsigned long*) (buffer + i));
        i += 4;
        for (int j = 0; j < 32; j++) {
          if (mask & (0x00000001 << j)) {
            if (j == 13) {
              *((short*) (buffer + i)) = 0;
            }
            
            m_cs->player.stats[j] = *((short*) (buffer + i));
            i += 2;
          }
        }
        break;
      }
      
      // EntityUpdate
      case 0x12: {
        unsigned int mask;
        int entity;
        
        for (;;) {
          mask = READ_CHAR;
          if (mask & 0x00000080) mask |= (READ_CHAR << 8);
          if (mask & 0x00008000) mask |= (READ_CHAR << 16);
          if (mask & 0x00800000) mask |= (READ_CHAR << 24);
          if (mask & 0x00000100) {
            entity = *((short*) (buffer + i));
            i += 2;
          } else {
            entity = READ_CHAR;
          }
          
          if (!entity)
            break;

          // Sanity check for entity identifier
          if (entity >= 1024) {
            getLogger()->error("Entity number greater than 1024! Protocol violation, aborting.");
          }
          
          m_cs->entities[entity].setVisible(true);
          if (mask & 0x00000800) m_cs->entities[entity].modelIndex =  READ_CHAR;
          if (mask & 0x00100000) m_cs->entities[entity].modelIndex2 = READ_CHAR;
          if (mask & 0x00200000) m_cs->entities[entity].modelIndex3 = READ_CHAR;
          if (mask & 0x00400000) m_cs->entities[entity].modelIndex4 =  READ_CHAR;
          if (mask & 0x00000010) m_cs->entities[entity].framenum = READ_CHAR;
          if (mask & 0x00020000) {
            m_cs->entities[entity].framenum = *((short*) (buffer + i));
            i += 2;
          }
          if (mask & 0x00010000) {
            if (mask & 0x02000000) {
              i += 4;
            } else {
              i++;
            }
          } else {
            if (mask & 0x02000000) {
              i += 2;
            }	
          }
          if (mask & 0x00004000) {
            if (mask & 0x00080000) {
              i+=4;
            } else {
              i++;
            }
          } else if (mask & 0x00080000) {
            i += 2;
          }
          if (mask & 0x00001000) {
            if (mask & 0x00040000) {
              m_cs->entities[entity].renderfx = *((int*) (buffer + i));
              i += 4;
            } else {
              m_cs->entities[entity].renderfx = READ_CHAR;
            }
          } else {
            if (mask & 0x00040000) {
              m_cs->entities[entity].renderfx = *((short*) (buffer + i));
              i += 2;
            }
          }
          if (mask & 0x00000001) {
            m_cs->entities[entity].origin[0] = 0.125 * ((float) *((short*) (buffer + i)));
            i += 2;
          }
          if (mask & 0x00000002) {
            m_cs->entities[entity].origin[1] = 0.125 * ((float) *((short*) (buffer + i)));
            i += 2;
          }
          if (mask & 0x00000200) {
            m_cs->entities[entity].origin[2] = 0.125 * ((float) *((short*) (buffer + i)));
            i += 2;
          }
          float f = 0.01 * (float) (timestamp - m_dataPoints[entity].timestamp);
          if (f > 0.0 && f <= 10.0) {
            m_cs->entities[entity].velocity[0] = (m_cs->entities[entity].origin[0] - m_dataPoints[entity].origin[0]) / f;
            m_cs->entities[entity].velocity[1] = (m_cs->entities[entity].origin[1] - m_dataPoints[entity].origin[1]) / f;
            m_cs->entities[entity].velocity[2] = (m_cs->entities[entity].origin[2] - m_dataPoints[entity].origin[2]) / f;
          } else {
            m_cs->entities[entity].velocity[0] = 0;
            m_cs->entities[entity].velocity[1] = 0;
            m_cs->entities[entity].velocity[2] = 0;
          }
          
          m_dataPoints[entity].timestamp = timestamp;
          m_dataPoints[entity].origin[0] = m_cs->entities[entity].origin[0];
          m_dataPoints[entity].origin[1] = m_cs->entities[entity].origin[1];
          m_dataPoints[entity].origin[2] = m_cs->entities[entity].origin[2];
          
          if (mask & 0x00000004) m_cs->entities[entity].angles[0] = (M_PI / 128.0 * (float) buffer[i++]);
          if (mask & 0x00000400) m_cs->entities[entity].angles[1] = (M_PI / 128.0 * (float) buffer[i++]);
          if (mask & 0x00000008) m_cs->entities[entity].angles[2] = (M_PI / 128.0 * (float) buffer[i++]);
          if (mask & 0x01000000) i += 6;
          if (mask & 0x04000000) i++;
          if (mask & 0x00000020) i++;
          if (mask & 0x08000000) i += 2;
          if (mask & 0x00000040) {
            m_cs->entities[entity].setVisible(false);
          }
        }
        break;
      }
      
      // FrameUpdate
      case 0x14: {
        int count;
        
        // Save current frame and parse updated frame
        m_lastFrame = m_currentFrame;
        m_currentFrame = *((unsigned long*) (buffer + i));
        i += 4;
        
        // Parse delta frame
        m_deltaFrame = *((unsigned long*) (buffer + i));
        i += 4;
        i += 1;
        
        count = buffer[i++];
        i += count;
        if (m_currentFrame - m_lastFrame > 12) {
          m_currentState = 0;
        } else {
          m_currentState = (m_currentState + m_currentFrame - m_lastFrame) % 16;
        }
        
        if (m_deltaFrame == 0xffffffff) {
          m_ds = &(m_gamestates[16]);
          m_packetLoss = 0;
        } else if (m_currentFrame - m_deltaFrame > 12) {
          getLogger()->warning("Too much packet loss!");
          m_packetLoss = 0x80000000;
          return 0;
        } else {
          m_ds = &(m_gamestates[(m_currentState + m_deltaFrame - m_currentFrame + 16) % 16]);
          m_packetLoss = 0;
        }
        
        m_cs = &(m_gamestates[m_currentState]);
        memcpy(m_cs, m_ds, sizeof(InternalGameState));
        break;
      }
      
      // Unknown packet type
      default: {
        getLogger()->warning(format("Received unknown packet type from server: 0x%02x") % (unsigned int) type);
        return 0;
      }
    }
  }
  
  // Check for misalignments
  if (i > length) {
    getLogger()->warning("Server packet misalignment error.");
    return 0;
  }
  
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
    // Unordered packet
    buffer[length + 8] = 0;
    sscanf((char *)(buffer + 4), "%s %d", b, &temp);

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
    // Packet with sequence number
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

