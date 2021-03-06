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
#include "mapping/grid.h"
#include "mapping/exporters.h"
#include "mapping/dynamic.h"
#include "planner/local.h"
#include "planner/global.h"
#include "dispatcher.h"
#include "rl/brains.h"

// States
#include "states/wander.h"
#include "states/swim.h"
#include "states/shoot.h"
#include "states/respawn.h"
#include "states/gotoammo.h"
#include "states/gotohealth.h"
#include "states/gotoweapon.h"
#include "states/gotoupgrade.h"
#include "states/dropweapon.h"
#include "states/camper.h"

// Voters
#include "voters/dummy.h"
#include "voters/droper.h"

// MOLD message bus
#include "mold/server.h"
#include "mold/client.h"

namespace HiveMind {

Context::Context(const std::string &id, const std::string &gamedir, const std::string &datadir, 
                 const std::string &skin, const std::string &mode, const std::string &knowledge, const std:: string &shootMode)
  : m_botId(id),
    m_gamedir(gamedir),
    m_datadir(datadir),
    m_connection(NULL),
    m_map(NULL),
    m_abort(false),
    m_dispatcher(new Dispatcher(this)),
    m_simulatorInitialized(false),
    m_skin(skin),
    m_mode(mode),
    m_knowledge(knowledge),
    m_shootMode(shootMode)
{
  Object::init();
  
  // Seed random generator
  srand(time(0));
  
  // Log our unique identifier
  getLogger()->info(format("Hivemind instance unique identifier is: %s") % id);
}

Context::~Context()
{
  delete m_connection;
  delete m_map;
}

void Context::simulationStart(const std::string &map)
{
  // Load maps
  getLogger()->info(format("Currently playing map: %s") % map);
  m_map = new Map(this, map);
  if (!m_map->open())
    getLogger()->error("Map open has failed, aborting now.");
  
  // Create dynamic mapping grid and load learned grid data
  m_grid = new Grid(m_map);
  std::string mn = std::string(basename(map.c_str()));
  mn = mn.substr(0, mn.find("."));
  m_grid->importGrid(getDataDir() + "/grid-" + mn + ".hm");
  
  // Create the dynamic mapper
  m_dynamicMapper = new DynamicMapper(this);
  
  // Create and start local planner
  m_localPlanner = new LocalPlanner(this);
  m_localPlanner->registerState(new WanderState(this));
  m_localPlanner->registerState(new SwimState(this));
  m_localPlanner->registerState(new ShootState(this));
  m_localPlanner->registerState(new RespawnState(this));
  m_localPlanner->registerState(new GoToAmmoState(this));
  m_localPlanner->registerState(new GoToHealthState(this));
  m_localPlanner->registerState(new GoToUpgradeState(this));
  m_localPlanner->registerState(new GoToWeaponState(this));
  m_localPlanner->registerState(new DropWeaponState(this));
  m_localPlanner->registerState(new CamperState(this));
  m_localPlanner->addEligibleState(m_localPlanner->getStateFromName("wander"));
  m_localPlanner->start();
  
  // Create and start global planner (only so that it is there, it won't
  // actually do anything in the simulation)
  m_globalPlanner = new GlobalPlanner(this);
  m_globalPlanner->start();
  
  m_simulatorInitialized = true;
}

void Context::simulateFrame(const GameState &state, Vector3f *orientation, Vector3f *velocity)
{
  if (!m_simulatorInitialized)
    return;
  
  // Process frame update from Quake II, update planner (local only as this
  // is a simulation)
  m_dynamicMapper->worldUpdated(state);
  m_localPlanner->worldUpdated(state);
  
  // Request next move from local planner
  bool fire;
  m_localPlanner->getBestMove(orientation, velocity, &fire);
}

void Context::connectTo(const std::string &host, unsigned int port)
{
  if (m_connection)
    return;
  
  m_connection = new Connection(this, m_botId, host, port, m_skin);
  m_connection->connect();
  
  // Load maps
  getLogger()->info(format("Currently playing map: %s") % m_connection->getMapName());
  m_map = new Map(this, m_connection->getMapName());
  if (!m_map->open())
    getLogger()->error("Map open has failed, aborting now.");
  
  // Create dynamic mapping grid and load learned grid data
  m_grid = new Grid(m_map);
  std::string mn = std::string(basename(m_connection->getMapName().c_str()));
  mn = mn.substr(0, mn.find("."));
  m_grid->importGrid(getDataDir() + "/grid-" + mn + ".hm");
  
  // Enter the game
  m_connection->begin();
}

void Context::execute()
{
  if (!m_connection || !m_connection->isOnline() || !m_map)
    getLogger()->error("Unable to start bot processing loop as we are not initialized!");
  
  // Create the dynamic mapper
  m_dynamicMapper = new DynamicMapper(this);
  
  // Create and start local planner
  m_localPlanner = new LocalPlanner(this);
  m_localPlanner->registerState(new WanderState(this));
  m_localPlanner->registerState(new SwimState(this));
  m_localPlanner->registerState(new ShootState(this));
  m_localPlanner->registerState(new RespawnState(this));
  m_localPlanner->registerState(new GoToAmmoState(this));
  m_localPlanner->registerState(new GoToHealthState(this));
  m_localPlanner->registerState(new GoToUpgradeState(this));
  m_localPlanner->registerState(new GoToWeaponState(this));
  m_localPlanner->registerState(new DropWeaponState(this));
  m_localPlanner->registerState(new CamperState(this));
  m_localPlanner->addEligibleState(m_localPlanner->getStateFromName("wander"));

  // Set learn to true if explore mode is set, otherwise exploit
  // the already gained knowledge
  m_localPlanner->getBrains()->setBrainMode(m_mode == "explore");
  m_localPlanner->start();
  
  // Create and start global planner
  m_globalPlanner = new GlobalPlanner(this);
  m_globalPlanner->registerVoter("System.PollTest", new DummyVoter());
  m_globalPlanner->registerVoter("System.WhoWillDrop", new DroperVoter(this));
  m_globalPlanner->start();
  
  while (!m_abort) {
    // Process frame update from Quake II server, update planner
    GameState state = m_connection->getGameState();
    m_connection->refreshInventory();
    m_dynamicMapper->worldUpdated(state);
    m_globalPlanner->worldUpdated(state);
    m_localPlanner->worldUpdated(state);
    
    // Deliver deferred events
    m_dispatcher->deliver();
    
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


