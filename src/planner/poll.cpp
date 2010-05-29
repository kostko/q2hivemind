/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "planner/poll.h"
#include "planner/global.h"
#include "context.h"
#include "logger.h"
#include "mold/client.h"
#include "planner/local.h"

#include <boost/foreach.hpp>
#include <boost/random.hpp>
#include <boost/lexical_cast.hpp>

// Protocol buffer messages
#include "src/mold/control.pb.h"


namespace HiveMind {

PollVote::PollVote()
  : m_bot(NULL),
    m_votes(0),
    m_choice("")
{
}

PollVote::PollVote(float votes, const std::string &choice)
  : m_bot(NULL),
    m_votes(votes),
    m_choice(choice)
{
}

PollVote::PollVote(Bot *bot, float votes, const std::string &choice)
  : m_bot(bot),
    m_votes(votes),
    m_choice(choice)
{
}

Poll::Poll(Context *context, Type type, timestamp_t duration, const std::string &category)
  : m_context(context),
    m_category(category),
    m_active(true),
    m_closesOn(Timing::getCurrentTimestamp() + duration),
    m_type(type)
{
  Object::init();
  
  // Create a list of eligible voters (these are all bots that have registered
  // with the directory at the time of poll creation)
  m_eligibleVoters = context->getGlobalPlanner()->getDirectory()->getRegisteredBots();
  
  // Generate a unique poll identifier
  boost::mt19937 rng;
  rng.seed(static_cast<unsigned int>(std::time(0)));
  m_pollId = boost::lexical_cast<std::string>(rng());
}

void Poll::addChoice(const std::string &choice)
{
  m_choices.insert(choice);
}

void Poll::addVote(const PollVote &vote)
{
  if (!m_active || m_eligibleVoters.find(vote.getVoter()) == m_eligibleVoters.end())
    return;
  
  if (m_type == VoteChoice && m_choices.find(vote.getChoice()) == m_choices.end())
    return;
  
  if (vote.getVotes() <= 0.0f)
    return;
  
  // Cast a vote
  m_votes[vote.getVoter()] = vote;
}

void Poll::process()
{
  if (!m_active)
    return;
  
  timestamp_t now = Timing::getCurrentTimestamp();
  
  // Check if poll has closed and toggle status
  if (now >= m_closesOn) {
    close();
    return;
  }
  
  // Check if all bots have already voted
  if (m_votes.size() >= m_eligibleVoters.size()) {
    close();
    return;
  }
}

void Poll::close()
{
  // Disable voting in this poll
  m_active = false;
  
  // Compute who the winner is
  if (m_type == VoteBot) {
    // Simply determine the bot with most votes
    Bot *bot = NULL;
    float votes = 0;
    
    typedef std::pair<Bot*, PollVote> BotVotePair;
    BOOST_FOREACH(BotVotePair p, m_votes) {
      if (bot == NULL || p.second.getVotes() > votes) {
        bot = p.first;
        votes = p.second.getVotes();
      }
    }
    
    m_winnerBot = bot;
    
    // Log winner
    if (bot) {
      if (m_category == "System.WhoWillDrop") {
        // Send a confirmation message to the winner
        MOLD::ClientPtr client = m_context->getMOLDClient();
        client->deliver(MOLD::Protocol::Message::DROP_CHOSEN, bot->getName());

        getLogger()->info(format("Sending a DROP_CHOSEN msg to %s.") % bot->getName());

        // TODO: if there is no winner, don't wait in camper state
      }
      getLogger()->info(format("Poll %s has closed, winner is %s with %f votes.") % m_pollId % bot->getName() % votes);
    }
    else
      getLogger()->info(format("Poll %s has closed, no candidates voted.") % m_pollId);
  } else if (m_type == VoteChoice) {
    // Determine the choice that received most votes
    boost::unordered_map<std::string, float> ballot;
    
    typedef std::pair<Bot*, PollVote> BotVotePair;
    BOOST_FOREACH(BotVotePair p, m_votes) {
      ballot[p.second.getChoice()] += p.second.getVotes();
    }
    
    float votes = 0;
    m_winnerChoice = "";
    
    typedef std::pair<std::string, float> BallotPair;
    BOOST_FOREACH(BallotPair p, ballot) {
      if (p.second > votes) {
        m_winnerChoice = p.first;
        votes = p.second;
      }
    }
    
    // Log winner
    getLogger()->info(format("Poll %s has closed, winning choice is %s.") % m_pollId % m_winnerChoice);
  }
}

}


