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

#define MAX_UPDATES 128

namespace HiveMind {

/**
 * A complete Quake 2 client implementation.
 */
class Connection : public Object {
public:
    /**
     * Class constructor.
     *
     * @param id A unique bot identifier
     * @param host Quake 2 server host
     * @param port Quake 2 server port
     */
    Connection(const std::string &id, const std::string &host, int port);
    
    /**
     * Class destructor.
     */
    virtual ~Connection();
    
    /**
     * Establishes a connection with the server.
     */
    void connect();
    
    /**
     * Notifies the server to start sending game updates.
     */
    void begin();
    
    /**
     * Disconnects from a server.
     */
    void disconnect();
    
    /**
     * Issues a move command. This method must be called on every
     * frame.
     *
     * @param angles Orientation vector
     * @param velocity Velocity vector
     * @param attack True to fire
     */
    void move(const Vector3f &angles, const Vector3f &velocity, bool attack);
    
    /**
     * Says something in global chat.
     *
     * @param msg Message to say
     */
    void say(const std::string &msg);
    
    /**
     * Returns true if the connection is currently established.
     */
    inline bool isConnected() const { return m_connected; }
    
    /**
     * Returns true if the connection is currently synced with server.
     */
    inline bool isOnline() const { return m_online; }
    
    /**
     * Writes to the server console in a blocking manner.
     *
     * @param msg Message to write
     */
    void writeConsoleSync(const std::string &msg);
    
    /**
     * Writes to the server console in a non-blocking manner.
     *
     * @param msg Message to write
     */
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
    
    /**
     * Dispatches a player update.
     */
    void dispatchUpdate();
    
    /**
     * Receives a packet from the server.
     *
     * @param buffer Destination buffer
     */
    int receivePacket(char *buffer);
    
    /**
     * Processes a received packet.
     *
     * @param data Packet contents
     * @param length Packet length
     */
    int processPacket(char *data, size_t length);
    
    /**
     * Sends an unordered unreliable data packet to server.
     */
    void sendUnorderedPacket(char *data, size_t length);
    
    /**
     * Sends an ordered unreliable data packet to server.
     */
    void sendUnreliablePacket(unsigned int seq, char *data, size_t length);
    
    /**
     * Sends an ordered reliable data packet to server.
     */
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
    
    // Updates
    class Update {
    public:
      EIGEN_MAKE_ALIGNED_OPERATOR_NEW
      
      Vector3f angles;
      Vector3f velocity;
      unsigned char msec, light, buttons, impulse;
      int timestamp;
    };
    
    Update m_updates[MAX_UPDATES];
    int m_currentUpdate;
    int m_lastUpdateTime;
};

}

#endif

