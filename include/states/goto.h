/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_STATES_GOTO_H
#define	HM_STATES_GOTO_H

#include "mapping/items.h"
#include "states/wander.h"
#include "timing.h"

#include <list>

namespace HiveMind {

class Context;

typedef std::pair<Item::Type, int> ItemValue;

enum {
    BETWEEN_GOTO = 20000   // The time interval between two state executions
};

/**
 * Compare two items by their values.
 */
bool item_cmp(ItemValue a, ItemValue b);

class GoToState : public WanderState {
public:
    /**
     * Constructor.
     */
    GoToState(Context *context, const std::string &name);

    /**
     * Prepare for entry into this state.
     *
     * @param metadata Supplied metadata
     */
    virtual void initialize(const boost::any &metadata);

    /**
     * This method should implement state specific event
     * checking, so the state can emit a signal when
     * needed. This method is called in main thread context.
     *
     * Default implementation does nothing.
     */
    virtual void checkEvent();

    /**
     * Prepare for leaving this state.
     */
    virtual void goodbye();

    /**
     * This method should implement state specific processing in
     * planning mode. This method is called in planner thread
     * context.
     */
    virtual void processPlanning();

    /**
     * Make this state one of the candidates for transition. This method is called in main thread context.
     */
    void makeEligible();

    /**
     * Sets the exists flag.
     */
    inline void setItemExists(bool exists) { m_exists = exists; }

    /**
     * Does at least one item exist?
     */
    inline bool itemExists() const { return m_exists; }

    /**
     * Requests path recomputation from current location to our
     * destination.
     *
     * @param randomize True means to pick the next node at random
     */
    void recomputePath(bool randomize = false);
protected:
    /**
     * Evaluates the items based on the current needs.
     */
    virtual void evaluate() = 0;

    // Values of items
    std::list<ItemValue> m_items;

    // Last time of state execution
    timestamp_t m_lastTime;

    // True if the bot knows for at least one useful item
    bool m_exists;

    // Should we seek a different item
    bool m_recompute;

    // The item we are currently looking for
    Item::Type m_currItem;
};

}

#endif
