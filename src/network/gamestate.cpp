/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "network/gamestate.h"

namespace HiveMind {

std::string Player::getWeaponName() {

  if (&weaponModel == NULL) return "Blaster";

  if (std::string::npos != weaponModel.find("v_blast")) return "Blaster";
  else if (std::string::npos != weaponModel.find("v_shotg")) return "Shotgun";
  else if (std::string::npos != weaponModel.find("v_shotg2")) return "Super Shotgun";
  else if (std::string::npos != weaponModel.find("v_machn")) return "Machinegun";
  else if (std::string::npos != weaponModel.find("v_chain")) return "Chaingun";
  else if (std::string::npos != weaponModel.find("v_launch")) return "Grenade Launcher";
  else if (std::string::npos != weaponModel.find("v_rocket")) return "Rocket Launcher";
  else if (std::string::npos != weaponModel.find("v_hyperb")) return "HyperBlaster";
  else if (std::string::npos != weaponModel.find("v_rail")) return "Railgun";
  else if (std::string::npos != weaponModel.find("v_bfg")) return "BFG10K";
  else if (std::string::npos != weaponModel.find("v_handgr")) return "Grenades";
  
}
}
