/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "context.h"
#include "logger.h"
#include "network/connection.h"
#include "mapping/map.h"
#include "planner/local.h"
#include "planner/global.h"
#include "dispatcher.h"

// States
#include "states/wander.h"
#include "states/swim.h"

// MOLD message bus
#include "mold/server.h"
#include "mold/client.h"

namespace HiveMind {

Context::Context(const std::string &id, const std::string &gamedir)
  : m_botId(id),
    m_gamedir(gamedir),
    m_connection(NULL),
    m_map(NULL),
    m_abort(false),
    m_dispatcher(new Dispatcher(this))
{
  Object::init();
  
  // Log our unique identifier
  getLogger()->info(format("Hivemind instance unique identifier is: %s") % id);
}

Context::~Context()
{
  delete m_connection;
  delete m_map;
}

void Context::connectTo(const std::string &host, unsigned int port)
{
  if (m_connection)
    return;
  
  m_connection = new Connection(m_botId, host, port);
  m_connection->connect();
  
  // Load maps
  getLogger()->info(format("Currently playing map: %s") % m_connection->getMapName());
  m_map = new Map(this, m_connection->getMapName());
  if (!m_map->open())
    getLogger()->error("Map open has failed, aborting now.");
  
  // Enter the game
  m_connection->begin();
}

void Context::execute()
{
  if (!m_connection || !m_connection->isOnline() || !m_map)
    getLogger()->error("Unable to start bot processing loop as we are not initialized!");
  
  // Create and start local planner
  m_localPlanner = new LocalPlanner(this);
  m_localPlanner->registerState(new WanderState(this));
  m_localPlanner->registerState(new SwimState(this));
  m_localPlanner->start();
  
  // Create and start global planner
  m_globalPlanner = new GlobalPlanner(this);
  m_globalPlanner->start();
  
  while (!m_abort) {
    // Process frame update from Quake II server, update planner
    GameState state = m_connection->getGameState();
    m_localPlanner->worldUpdated(state);
    
    // Request next move from local planner
    Vector3f orientation, velocity;
    bool fire;
    m_localPlanner->getBestMove(&orientation, &velocity, &fire);
    
    // Send move to Quake II server
    m_connection->move(orientation, velocity, fire);
  }
  
  getLogger()->warning("Processing loop aborted.");
  
  // Reset abort flag
  m_abort = false;
}

void Context::runMOLDBus(const std::string &address)
{
  getLogger()->info("Attempting to run MOLD message bus server...");
  
  // Create the async IO service and run the server (port 8472)
  try {
    boost::asio::io_service service;
    tcp::endpoint endpoint(boost::asio::ip::address::from_string(address), 8472);
    MOLD::ServerPtr server(new MOLD::Server(service, endpoint));
    service.run();
  } catch (std::exception &e) {
    getLogger()->error("Invalid MOLD listen address specified! Enter a valid IPv4/IPv6 address.");
  }
}

void Context::runMOLDClient(const std::string &address)
{
  getLogger()->info(format("Spawning MOLD message bus client and connecting to '%s'...") % address);
  
  // Create the async IO service and run the client
  try {
    tcp::resolver resolver(m_moldClientService);
    tcp::resolver::query query(address, "8472");
    tcp::resolver::iterator endpoints = resolver.resolve(query);
    m_moldClient = MOLD::ClientPtr(new MOLD::Client(this, m_moldClientService, endpoints));
    
    // Spawn the thread for handling communications
    m_moldClientThread = boost::thread(boost::bind(&boost::asio::io_service::run, &m_moldClientService));
  } catch (std::exception &e) {
    getLogger()->error("Failed to initialize MOLD message bus client!");
  }
}

}


