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

#include <boost/thread.hpp>

namespace HiveMind {

class Connection : public Object {
public:
    Connection(const std::string &id, const std::string &host, int port);
    
    virtual ~Connection();
    
    void connect();
    
    /**
     * Returns true if the connection is currently established.
     */
    inline bool isConnected() const { return m_connected; }
protected:
    /**
     * Entry point for the internal worker thread.
     */
    void worker();
    
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
    
    // Processing thread
    boost::thread m_workerThread;
    boost::mutex m_gameStateMutex;
    boost::mutex m_sendURMutex;
    boost::mutex m_sendRLMutex;
    
    // State
    bool m_connected;
    unsigned int m_svSequence;
    unsigned int m_clSequence;
    unsigned int m_svBit;
    unsigned int m_clBit;
    unsigned int m_lastReliableSeq;
    bool m_reliableReceived;
    int m_lastPingTime;
    int m_runningPing;
    unsigned int m_challengeNum;
    unsigned int m_clientId;
};

}

#endif

