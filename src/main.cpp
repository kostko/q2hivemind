/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "context.h"

#include <ctime>

#include <boost/program_options.hpp>
#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>

using namespace HiveMind;
namespace po = boost::program_options;

/**
 * Hivemind entry point.
 */
int main(int argc, char **argv)
{
  // Parse program options
  po::options_description desc("Allowed options");
  desc.add_options()
    ("help", "show help message")
    ("mold-server", po::value<std::string>(), "run MOLD message bus server")
    ("mold-client", po::value<std::string>(), "connect to MOLD message bus")
    ("quake2-server", po::value<std::string>()->default_value("::1"), "connection to specified quake2 server")
    ("quake2-dir", po::value<std::string>()->default_value("/usr/share/games/quake2"), "specify quake2 directory")
  ;
  
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  } catch (std::exception &e) {
    std::cout << "ERROR: There is an error in your syntax!" << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }
  
  // Display help when requested
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }
  
  // Generate a unique identifier
  boost::mt19937 rng;
  rng.seed(static_cast<unsigned int>(std::time(0)));
  std::string uniqueId = boost::lexical_cast<std::string>(rng());
  
  // Create a context
  Context context("h" + uniqueId, vm["quake2-dir"].as<std::string>());
  
  // Start MOLD server when requested
  if (vm.count("mold-server")) {
    context.runMOLDBus(vm["mold-server"].as<std::string>());
  } else if (vm.count("mold-client")) {
    context.runMOLDClient(vm["mold-client"].as<std::string>());
    context.connectTo(vm["quake2-server"].as<std::string>(), 27910);
    context.execute();
  } else {
    std::cout << "ERROR: Please specify --mold-server or --mold-client!" << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }
  
  return 0;
}


