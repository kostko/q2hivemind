/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_PLANNER_POLL_H
#define HM_PLANNER_POLL_H

#include "object.h"
#include "timing.h"
#include "planner/directory.h"

#include <boost/unordered_set.hpp>

namespace HiveMind {

namespace MOLD {
  namespace Protocol {
    class Message;
  }
}

class Context;

class PollVote {
public:
    /**
     * Constructs an invalid vote.
     */
    PollVote();
    
    /**
     * Class constructor.
     *
     * @param votes Number of votes
     * @param choice Optional choice (for choice polls)
     */
    PollVote(float votes, const std::string &choice = "");
    
    /**
     * Class constructor.
     *
     * @param bot Bot that cast the vote
     * @param votes Number of votes
     * @param choice Optional choice (for choice polls)
     */
    PollVote(Bot *bot, float votes, const std::string &choice = "");
    
    /**
     * Returns the voter bot instance.
     */
    inline Bot *getVoter() const { return m_bot; }
    
    /**
     * Returns the voter's choice.
     */
    inline std::string getChoice() const { return m_choice; }
    
    /**
     * Returns the number of voter's votes.
     */
    inline float getVotes() const { return m_votes; }
private:
    Bot *m_bot;
    std::string m_choice;
    float m_votes;
};

/**
 * Team poll.
 */
class Poll : public Object {
public:
    /**
     * Valid voting types:
     *   VoteChoice - bots vote for different choices
     *   VoteBot - bots vote for themselves
     */
    enum Type {
      VoteChoice,
      VoteBot
    };
    
    /**
     * Class constructor.
     *
     * @param context Bot context
     * @param type Poll type
     * @param duration Poll duration (in msec)
     * @param category Voting category
     */
    Poll(Context *context, Type type, timestamp_t duration, const std::string &category);
    
    /**
     * Returns the unique poll identifier.
     */
    inline std::string getId() const { return m_pollId; }
    
    /**
     * Returns the poll category.
     */
    inline std::string getCategory() const { return m_category; }
    
    /**
     * Returns true if this poll is currently active.
     */
    inline bool isActive() const { return m_active; } 
    
    /**
     * Adds a new valid choice to this poll. Only makes sense for
     * choice polls.
     */
    void addChoice(const std::string &choice);
    
    /**
     * Adds a vote into this poll.
     */
    void addVote(const PollVote &vote);
    
    /**
     * Performs poll processing.
     */
    void process();
    
    /**
     * Returns this poll's expiry timestamp.
     */
    inline timestamp_t getExpiryTimestamp() const { return m_closesOn; }
    
    /**
     * Returns the winning bot. Only makes sense for bot polls.
     */
    inline Bot *getWinnerBot() const { return m_winnerBot; }
    
    /**
     * Returns the winning choice. Only makes sense for choice polls.
     */
    std::string getWinnerChoice() const { return m_winnerChoice; }
protected:
    void close();
private:
    // Context
    Context *m_context;
    
    // Unique poll identifier and status
    std::string m_category;
    std::string m_pollId;
    bool m_active;
    timestamp_t m_closesOn;
    Type m_type;
    
    // Eligible voters
    BotSet m_eligibleVoters;
    
    // Choices and votes
    boost::unordered_set<std::string> m_choices;
    boost::unordered_map<Bot*, PollVote> m_votes;
    
    // Winners
    std::string m_winnerChoice;
    Bot *m_winnerBot;
};

/**
 * A voter interface.
 */
class PollVoter {
public:
    /**
     * Class constructor.
     */
    PollVoter()
    {}
    
    /**
     * Class destructor.
     */
    virtual ~PollVoter()
    {}
    
    /**
     * This method should return the vote for the specified poll.
     *
     * @param requestor Bot requesting the poll
     * @param category Poll category
     * @return A valid PollVote
     */ 
    virtual PollVote vote(Bot *requestor, const std::string &category) = 0;
};

}

#endif

