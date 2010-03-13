/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "logger.h"

#include <iostream>
#include <time.h>
#include <stdlib.h>

using namespace std;
using boost::format;

namespace HiveMind {

Logger::Logger(const std::string &name)
  : m_module(name)
{
}

void Logger::error(const std::string &message)
{
  log("ERROR  ", message);
  abort();
}

void Logger::error(const boost::basic_format<char, std::char_traits<char>, std::allocator<char> > &message)
{
  Logger::error(str(message));
}

void Logger::warning(const std::string &message)
{
  log("WARNING", message);
}

void Logger::warning(const boost::basic_format<char, std::char_traits<char>, std::allocator<char> > &message)
{
  Logger::warning(str(message));
}

void Logger::info(const std::string &message)
{
  log("INFO   ", message);
}

void Logger::info(const boost::basic_format<char, std::char_traits<char>, std::allocator<char> > &message)
{
  Logger::info(str(message));
}

void Logger::log(const std::string &type, const std::string &message)
{
  time_t rawtime;
  struct tm *timeinfo;
  
  // Grab formatted date and time
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(m_timeBuffer, 80, "%a, %d %b %Y %H:%M:%S", timeinfo);
  
  // Output string to stdout
  cout << m_timeBuffer << " " << type << " [" << m_module << "] " << message << endl;
}


}

