/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_VOTERS_DUMMY_H
#define HM_VOTERS_DUMMY_H

#include "planner/poll.h"

namespace HiveMind {

/**
 * A dummy voter for test purpuses.
 */
class DummyVoter : public PollVoter {
public:
    /**
     * This method should return the vote for the specified poll.
     *
     * @param requestor Bot requesting the poll
     * @param category Poll category
     * @return A valid PollVote
     */ 
    PollVote vote(Bot *requestor, const std::string &category);
};

}

#endif

