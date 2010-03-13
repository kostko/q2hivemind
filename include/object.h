/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_OBJECT_H
#define HM_OBJECT_H

#include "globals.h"

#include <string>

namespace HiveMind {

class Logger;

class Object {
public:
    /**
     * Class constructor.
     */
    Object();
    
    /**
     * Class destructor.
     */
    virtual ~Object();
    
    /**
     * Returns the class name of this object.
     */
    std::string getClassName() const;
protected:
    /**
     * This method must be run from parent constructor.
     */
    void init();
    
    /**
     * A convenience method to get the logger for this
     * object.
     */
    inline Logger *getLogger() const { return m_logger; }
private:
    // Logger instance
    Logger *m_logger;
};

}

#endif

