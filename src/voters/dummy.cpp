/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "voters/dummy.h"
#include "algebra.h"

namespace HiveMind {

PollVote DummyVoter::vote(Bot *requestor, const std::string &category)
{
  return PollVote(Algebra::randFloat() * 100.0f);
}

}


