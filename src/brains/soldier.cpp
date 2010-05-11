/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "logger.h"
#include "brains/soldier.h"
 
namespace HiveMind {

SoldierBrains::SoldierBrains(HiveMind::LocalPlanner *planner)
  : Brains(planner)
{
  vector<int> stateComps;
  stateComps.push_back(1);
  
  vector<int> actionComps;
  actionComps.push_back(1);
  
  Brains::init(stateComps, actionComps);
  
  getLogger()->info("Brains initialized.");
}

SoldierBrains::~SoldierBrains()
{
}

double SoldierBrains::reward(BrainState *prevState, BrainState *currState)
{
  //TODO
  return 0;
}

BrainState *SoldierBrains::observe()
{
  //TODO
  return NULL;
}

void SoldierBrains::execute(BrainAction *action)
{
  //TODO
}

}
