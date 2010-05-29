/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/global.h"
#include "context.h"
#include "logger.h"
#include "mold/client.h"
#include "planner/directory.h"
#include "planner/poll.h"
#include "planner/local.h"
#include "network/connection.h"
#include "dispatcher.h"
#include "event.h"

// Protocol buffer messages
#include "src/mold/control.pb.h"

#include <sstream>
#include <ctime>
#include <boost/foreach.hpp>

using namespace HiveMind::MOLD;

namespace HiveMind {

GlobalPlanner::GlobalPlanner(Context *context)
  : m_context(context),
    m_directory(new Directory(context)),
    m_lastCollection(0),
    m_lastBotUpdate(0),
    m_lastPollTest(Timing::getCurrentTimestamp())
{
  Object::init();
  
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (client) {
    // Connect to message received signal
    client->signalMessageReceived.connect(boost::bind(&GlobalPlanner::moldMessageReceived, this, _1));
  }
}

GlobalPlanner::~GlobalPlanner()
{
}

void GlobalPlanner::start()
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (!client) {
    // The client might not be available when we are inside the simulation
    // so in this case we simply ignore this and return
    return;
  }
  
  if (!client->isConnected()) {
    client->signalConnected.connect(boost::bind(&GlobalPlanner::start, this));
    return;
  }
  
  // We have a connection with MOLD message bus, send an annoucement to everyone
  Protocol::Announcement ann;
  ann.set_name(m_context->getBotId());
  ann.set_entityid(m_context->getConnection()->getPlayerEntityId());
  ann.set_reply(false);
  client->deliver(Protocol::Message::CONTROL_ANNOUNCE, &ann);
  
  // Subscribe to some bot-related events
  Dispatcher *dispatcher = m_context->getDispatcher();
  dispatcher->signalBotRespawn.connect(boost::bind(&GlobalPlanner::botRespawned, this, _1));
  dispatcher->signalEntityUpdated.connect(boost::bind(&GlobalPlanner::entityUpdated, this, _1));
}

void GlobalPlanner::worldUpdated(const GameState &state)
{
  timestamp_t now = Timing::getCurrentTimestamp();
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (!client) {
    // The client might not be available when we are inside the simulation
    // so in this case we simply ignore this and return
    return;
  }
  
  // Collect out of date bots
  if (now - m_lastCollection > bot_collect_interval) {
    m_directory->collect();
    m_lastCollection = now;
  }
  
  // Emit our update (this must always be the server origin not an interpolated one
  // as interpolated origins might be inside walls or off ledges)
  Vector3f origin = state.player.serverOrigin;
  
  if (origin != m_lastBotOrigin || now - m_lastBotUpdate > bot_update_interval) {
    // Construct the location update event
    Protocol::LocationUpdate upd;
    upd.set_x(origin[0]);
    upd.set_y(origin[1]);
    upd.set_z(origin[2]);
    client->deliver(Protocol::Message::EVENT_LOCATION_UPDATE, &upd);
    
    // Reset timer and last bot origin
    m_lastBotUpdate = now;
    m_lastBotOrigin = origin;
  }
  
  // Process active polls
  std::list<std::string> clearQueue;
  typedef std::pair<std::string, Poll*> PollPair;
  BOOST_FOREACH(PollPair p, m_polls) {
    Poll *poll = p.second;
    poll->process();
    
    // Check if voting has been completed and in this case emit proper event then
    // mark poll for removal
    if (!poll->isActive()) {
      PollVoteCompletedEvent event(poll);
      m_context->getDispatcher()->emit(&event);
      clearQueue.push_back(p.first);
    }
  }
  
  // Mutex must be held while erasing polls as events might still be delivered
  // while the polls are being deleted
  {
    boost::lock_guard<boost::mutex> g(m_pollMutex);
    BOOST_FOREACH(std::string pollId, clearQueue) {
      delete m_polls[pollId];
      m_polls.erase(pollId);
    }
  }
  
  // Test the voting system once in a while
  if (now - m_lastPollTest >= 60000) {
    createPoll(new Poll(m_context, Poll::VoteBot, 5000, "System.PollTest"));
    m_lastPollTest = now;
  }
}

Bot *GlobalPlanner::getBotOrRequestAnnounce(const MOLD::Protocol::Message &msg)
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  Bot *bot = m_directory->getBotByName(msg.sourceid());
  if (!bot) {
    // Bot not yet available in our directory, request annoucement first
    getLogger()->warning(format("Got an event from an unknown bot (%s), requesting annoucement.") % msg.sourceid());
    
    // Construct an announcement and send it
    Protocol::Announcement ann;
    ann.set_name(m_context->getBotId());
    ann.set_entityid(m_context->getConnection()->getPlayerEntityId());
    ann.set_reply(false);
    client->deliver(Protocol::Message::CONTROL_ANNOUNCE, &ann, msg.sourceid());
  }
  
  return bot;
}

void GlobalPlanner::moldMessageReceived(const Protocol::Message &msg)
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (!client) {
    // If we are here that means that the client has been destroyed
    // between updates, this is a problem and should be logged
    getLogger()->warning("MOLD client not available, but message received!");
    return;
  }
  
  switch (msg.type()) {
    case Protocol::Message::CONTROL_ANNOUNCE: {
      // Received new bot announcement
      Protocol::Announcement ann = message_cast<Protocol::Announcement>(msg);
      Bot *bot = m_directory->getBotByName(ann.name());
      if (!bot) {
        getLogger()->info(format("A new bot (%s) with entity id %d has joined us.") % ann.name() % ann.entityid());
        
        // Insert new bot into directory
        m_directory->registerBot(ann.name(), ann.entityid());
      } else {
        // Bot is already known, simply update its timestamp,
        bot->updateTime();
      }
      
      // If this is not a reply, we should announce ourselves to the bot as well
      if (!ann.reply()) {
        Protocol::Announcement rply;
        rply.set_name(m_context->getBotId());
        rply.set_entityid(m_context->getConnection()->getPlayerEntityId());
        rply.set_reply(true);
        client->deliver(Protocol::Message::CONTROL_ANNOUNCE, &rply, ann.name());
      }
      break;
    }
    
    case Protocol::Message::EVENT_LOCATION_UPDATE: {
      // Location update event
      Protocol::LocationUpdate upd = message_cast<Protocol::LocationUpdate>(msg);
      Bot *bot = getBotOrRequestAnnounce(msg);
      if (bot) {
        // Update bot's origin and update time
        bot->setOrigin(Vector3f(upd.x(), upd.y(), upd.z()));
        bot->updateTime();
        
        // Generate an update event
        BotLocationUpdateEvent event(bot, bot->getOrigin());
        m_context->getDispatcher()->emit(&event);
      }
      break;
    }
    
    case Protocol::Message::EVENT_RESPAWNED: {
      // Bot respawn event
      Bot *bot = getBotOrRequestAnnounce(msg);
      if (bot) {
        // Generate a local respawn event
        BotRespawnEvent event(bot);
        m_context->getDispatcher()->emit(&event);
      }
      break;
    }
    
    case Protocol::Message::EVENT_ENTITY_UPDATE: {
      // Entity update event
      Protocol::EntityUpdate upd = message_cast<Protocol::EntityUpdate>(msg);
      Entity entity;
      entity.serverOrigin = Vector3f(upd.x(), upd.y(), upd.z());
      entity.origin = entity.serverOrigin;
      entity.modelIndex = upd.modelindex();
      entity.setEntityId(upd.entityid());
      entity.setPlayer(upd.player());
      entity.setVisible(true);
      
      EntityUpdatedEvent event(entity, true);
      m_context->getDispatcher()->emit(&event);
      break;
    }
    
    case Protocol::Message::POLL_CREATE: {
      // Poll create event
      Bot *bot = getBotOrRequestAnnounce(msg);
      if (bot) {
        // Check to see if we have a voter registered for this purpuse and let it
        // vote immediately
        Protocol::PollCreate pcr = message_cast<Protocol::PollCreate>(msg);
        if (m_pollVoters.find(pcr.category()) != m_pollVoters.end()) {
          // Request the voter to vote immediately
          PollVoter *voter = m_pollVoters[pcr.category()];
          PollVote vote = voter->vote(bot, pcr.category());
          
          // Dispatch the vote back
          Protocol::PollVote vt;
          vt.set_pollid(pcr.pollid());
          vt.set_choice(vote.getChoice());
          vt.set_votes(vote.getVotes());
          client->deliver(Protocol::Message::POLL_VOTE, &vt, msg.sourceid());
          
          // Log voting
          getLogger()->info(format("Voted [%f] in poll '%s' in category '%s'.") % vt.votes() % pcr.pollid() % pcr.category());
        } else {
          // Unknown vote category, we can do nothing
          getLogger()->warning(format("Bot has requested a poll on '%s', but we are missing a voter!") % pcr.category());
        }
      }
      break;
    }
    
    case Protocol::Message::POLL_VOTE: {
      // Poll vote event
      Bot *bot = getBotOrRequestAnnounce(msg);
      if (bot) {
        // Check to see if we have a running poll under this identifier
        Protocol::PollVote vt = message_cast<Protocol::PollVote>(msg);
        boost::lock_guard<boost::mutex> g(m_pollMutex);
        
        if (m_polls.find(vt.pollid()) != m_polls.end()) {
          // A poll has been found, let's vote
          m_polls[vt.pollid()]->addVote(PollVote(bot, vt.votes(), vt.choice()));
        }
      }
      break;
    }

    case Protocol::Message::DROP_CHOSEN: {
      Bot *bot = getBotOrRequestAnnounce(msg);
      if (bot) {
        getLogger()->info(format("I HAVE RECEIVED DROP_CHOSEN MOLD MESSAGE from %s. YUHUUUU!!!!") % bot->getName());

        // Generate a local drop weapon event
        GoToAndDropWeaponEvent event(bot);
        m_context->getDispatcher()->emit(&event);
        
      }
      break;
    }

    case Protocol::Message::STOP_WAITING_FOR_DROP: {
      Bot *bot = getBotOrRequestAnnounce(msg);
      if (bot) {
        getLogger()->info(format("I HAVE RECEIVED STOP_WAITING_FOR_DROP MOLD MESSAGE from %s. YUHUUUU!!!!") % bot->getName());
        m_context->getLocalPlanner()->requestTransition("wander");
      }
      break;
    }
    default: {
      // Message code not recongnized, emit a warning
      getLogger()->warning("Unrecognized MOLD message received from bus!");
      break;
    }
  }
}

void GlobalPlanner::botRespawned(BotRespawnEvent *event)
{
  if (!event->getBot()) {
    // Only forward events generated by the local bot
    MOLD::ClientPtr client = m_context->getMOLDClient();
    client->deliver(Protocol::Message::EVENT_RESPAWNED);
  }
}

void GlobalPlanner::entityUpdated(EntityUpdatedEvent *event)
{
  // Do not repropagate external entity updates
  if (event->isExternal())
    return;
  
  Entity entity = event->getEntity();
  Connection *conn = m_context->getConnection();
  
  if (entity.isVisible() && entity.modelIndex > 0) {
    // Do not emit events for other bot entities in the team as those are updated
    // via bot location events anyway
    if (entity.isPlayer() && m_directory->isFriend(entity.getEntityId())) {
      return;
    }
    
    // Construct and transmit an entity update via MOLD
    MOLD::ClientPtr client = m_context->getMOLDClient();
    Protocol::EntityUpdate upd;
    upd.set_x(entity.serverOrigin[0]);
    upd.set_y(entity.serverOrigin[1]);
    upd.set_z(entity.serverOrigin[2]);
    upd.set_entityid(entity.getEntityId());
    upd.set_modelindex(entity.modelIndex);
    upd.set_player(entity.isPlayer());
    client->deliver(Protocol::Message::EVENT_ENTITY_UPDATE, &upd);
  }
}

void GlobalPlanner::createPoll(Poll *poll)
{
  MOLD::ClientPtr client = m_context->getMOLDClient();
  if (!client || !client->isConnected()) {
    delete poll;
    return;
  }
  
  getLogger()->info(format("Starting a new poll %s for category '%s'.") % poll->getId() % poll->getCategory());
  m_polls[poll->getId()] = poll;
  
  // Notify other members that this poll is on
  Protocol::PollCreate msg;
  msg.set_pollid(poll->getId());
  msg.set_closeson(poll->getExpiryTimestamp());
  msg.set_category(poll->getCategory());
  client->deliver(Protocol::Message::POLL_CREATE, &msg);
}

void GlobalPlanner::registerVoter(const std::string &category, PollVoter *voter)
{
  getLogger()->info(format("Registering poll voter for category '%s'...") % category);
  m_pollVoters[category] = voter;
}

}


