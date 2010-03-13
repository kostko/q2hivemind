/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "object.h"
#include "logger.h"

#include <typeinfo>
#include <cxxabi.h>

using namespace std;

namespace HiveMind {

Object::Object()
{
}

Object::~Object()
{
}

void Object::init()
{
  // We must create this here as if the method is run from Object then
  // RTTI will give incorrect class information
  m_logger = new Logger(getClassName());
}

string Object::getClassName() const
{
  size_t len;
  int s;
  char *p = abi::__cxa_demangle(typeid(*this).name(), 0, &len, &s);
  return string(p);
}

}

