/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_CONTEXT_H
#define HM_CONTEXT_H

#include "globals.h"
#include "object.h"

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace HiveMind {

namespace MOLD {
  class Client;
  typedef boost::shared_ptr<Client> ClientPtr;
}

class Connection;
class Map;
class LocalPlanner;
class GlobalPlanner;
class Dispatcher;

class Context : public Object {
public:
    /**
     * Class constructor.
     *
     * @param id Unique bot identifier
     * @param gamedir Quake 2 resource directory
     */
    Context(const std::string &id, const std::string &gamedir);
    
    /**
     * Class destructor.
     */
    virtual ~Context();
    
    /**
     * Returns the Quake 2 resource directory location.
     */
    inline std::string getGameDir() const { return m_gamedir; }
    
    /**
     * Returns this bot's identifier.
     */
    inline std::string getBotId() const { return m_botId; }
    
    /**
     * Returns the connection instance associated with this
     * context.
     */
    inline Connection *getConnection() const { return m_connection; }
    
    /**
     * Returns the current map instance.
     */
    inline Map *getMap() const { return m_map; }
    
    /**
     * Returns the local planner instance for this bot.
     */
    inline LocalPlanner *getLocalPlanner() const { return m_localPlanner; }
    
    /**
     * Returns the global planner instance for this bot.
     */
    inline GlobalPlanner *getGlobalPlanner() const { return m_globalPlanner; }
    
    /**
     * Returns the event dispatcher instance for this bot.
     */
    inline Dispatcher *getDispatcher() const { return m_dispatcher; }
    
    /**
     * Establishes a connection with the specified Quake 2 server.
     */
    void connectTo(const std::string &host, unsigned int port);
    
    /**
     * Enters the central bot processing loop.
     */
    void execute();
    
    /**
     * Raise the abort flag.
     */
    inline void abort() { m_abort = true; }
    
    /**
     * Runs the MOLD message bus server.
     *
     * @param address Address to bind to
     */
    void runMOLDBus(const std::string &address);
    
    /**
     * Runs the MOLD client.
     *
     * @param address Address to connect to
     */
    void runMOLDClient(const std::string &address);
    
    /**
     * Returns the MOLD client instance.
     */
    inline MOLD::ClientPtr getMOLDClient() const { return m_moldClient; }
private:
    // Unique bot identifier and game directory
    std::string m_botId;
    std::string m_gamedir;
    
    // Connection to Quake 2 server
    Connection *m_connection;
    
    // Current map
    Map *m_map;
    
    // Abort request flag
    bool m_abort;
    
    // Planners
    LocalPlanner *m_localPlanner;
    GlobalPlanner *m_globalPlanner;
    
    // Event dispatcher
    Dispatcher *m_dispatcher;
    
    // MOLD message bus
    boost::thread m_moldClientThread;
    boost::asio::io_service m_moldClientService;
    MOLD::ClientPtr m_moldClient;
};

}

#endif

