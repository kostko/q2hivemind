/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_RL_BRAINSTATE_H
#define HM_RL_BRAINSTATE_H

#include "rl/enumvector.h"
#include <string>

namespace HiveMind {

/**
 * A state of the bot's brain.
 */
class BrainState : public EnumVector {
public:
    /**
     * Constructors.
     */ 
    BrainState();
    BrainState(const std::string &name);
    
    /**
     * Destructor.
     */
    ~BrainState() {}
    
    /**
     * Init.
     * @param components components[i] is the number of states that the i-th component can take.
     *                   Note: this also means that the values of component i go from 0 to components[i]-1!
     */
    void init(std::vector<int> &components);
    
    /**
     * Action name.
     */
    inline std::string getName() const { return m_name; }

private:
  std::string m_name;
};

}

#endif
