/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_NETWORK_CONNECTION_H
#define HM_NETWORK_CONNECTION_H

#include "object.h"
#include "network/gamestate.h"

#include <list>
#include <boost/thread.hpp>

namespace HiveMind {

class Connection : public Object {
public:
    Connection(const std::string &id, const std::string &host, int port);
    
    virtual ~Connection();
    
    void connect();
    
    void begin();
    
    /**
     * Returns true if the connection is currently established.
     */
    inline bool isConnected() const { return m_connected; }
    
    /**
     * Returns true if the connection is currently synced with server.
     */
    inline bool isOnline() const { return m_online; }
    
    void writeConsoleSync(const std::string &msg);
    
    void writeConsoleAsync(const std::string &msg);
protected:
    /**
     * Entry point for the internal protocol worker thread.
     */
    void workerProtocol();
    
    /**
     * Entry point for the internal console worker thread.
     */
    void workerConsole();
    
    int receivePacket(char *buffer);
    
    int processPacket(char *data, size_t length);
    
    void sendUnorderedPacket(char *data, size_t length);
    
    void sendUnreliablePacket(unsigned int seq, char *data, size_t length);
    
    void sendReliablePacket(unsigned int seq, char *data, size_t length);
private:
    // Server information
    std::string m_host;
    std::string m_port;
    
    // Configuration
    StringMap m_config;
    
    // Socket
    int m_socket;
    
    // Processing threads and locks
    boost::thread m_workerThread;
    boost::thread m_consoleThread;
    boost::mutex m_gameStateMutex;
    boost::mutex m_sendURMutex;
    boost::mutex m_sendRLMutex;
    boost::mutex m_consoleMutex;
    
    // Console message queue
    std::list<std::string> m_consoleQueue;
    boost::condition_variable m_consoleCond;
    
    // Precache notification
    boost::mutex m_onlineMutex;
    boost::condition_variable m_onlineCond;
    
    // State
    bool m_connected;
    bool m_online;
    unsigned int m_svSequence;
    unsigned int m_clSequence;
    unsigned int m_svBit;
    unsigned int m_clBit;
    unsigned int m_lastReliableSeq;
    bool m_reliableReceived;
    int m_lastPingTime;
    int m_runningPing;
    unsigned int m_challengeNum;
    unsigned short m_clientId;
    
    // Gamestate
    unsigned int m_currentState;
    unsigned int m_currentFrame;
    unsigned int m_lastFrame;
    unsigned int m_deltaFrame;
    unsigned int m_packetLoss;
    InternalGameState m_gamestates[17];
    InternalGameState *m_cs;
    InternalGameState *m_ds;
    InternalGameState *m_spawn;
    TimePoint m_dataPoints[1024];
    
    // Inventory
    int m_inventory[256];
    unsigned int m_lastInventoryUpdate;
    
    // Server information
    std::string m_serverConfig[1568];
    int m_serverVersion;
    int m_maxPlayers;
    int m_playerNum;
    int m_loginKey;
};

}

#endif

