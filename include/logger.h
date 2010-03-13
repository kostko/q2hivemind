/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_LOGGER_H
#define HM_LOGGER_H

#include "globals.h"

#include <string>
#include <boost/format.hpp>

namespace HiveMind {

class Logger {
public:
    /**
     * Class constructor.
     */
    Logger(const std::string &name);
    
    /**
     * Output an error message.
     *
     * @param message Message string
     */
    void error(const std::string &message);
    
    /**
     * Output an error message.
     *
     * @param message Message
     */
    void error(const boost::basic_format<char, std::char_traits<char>, std::allocator<char> > &message);
    
    /**
     * Output a warning message.
     *
     * @param message Message string
     */
    void warning(const std::string &message);
    
    /**
     * Output a warning message.
     *
     * @param message Message
     */
    void warning(const boost::basic_format<char, std::char_traits<char>, std::allocator<char> > &message);
    
    /**
     * Output an info message.
     *
     * @param message Message string
     */
    void info(const std::string &message);
    
    /**
     * Output an info message.
     *
     * @param message Message
     */
    void info(const boost::basic_format<char, std::char_traits<char>, std::allocator<char> > &message);
    
    /**
     * Generic log method.
     *
     * @param type Log type
     * @param message Message string
     */
    void log(const std::string &type, const std::string &message);
private:
    // Currently active module identifier
    std::string m_module;
    
    // Time info buffer
    char m_timeBuffer[80];
};

}

#endif

