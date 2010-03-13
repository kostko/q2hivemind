/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_GLOBALS_H
#define HM_GLOBALS_H

#include <boost/unordered_map.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Array>

USING_PART_OF_NAMESPACE_EIGEN

namespace HiveMind {

// Global definitions for ease of use
typedef boost::unordered_map<std::string, std::string> StringMap;
typedef std::pair<std::string, std::string> StringPair;

}

#endif

