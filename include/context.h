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

namespace HiveMind {

class Connection;

class Context : public Object {
public:
    /**
     * Class constructor.
     *
     * @param id Unique bot identifier
     */
    Context(const std::string &id);
    
    /**
     * Class destructor.
     */
    virtual ~Context();
    
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
     * Establishes a connection with the specified Quake 2 server.
     */
    void connectTo(const std::string &host, unsigned int port);
private:
    // Unique bot identifier
    std::string m_botId;
    
    // Connection to Quake 2 server
    Connection *m_connection;
};

}

#endif

