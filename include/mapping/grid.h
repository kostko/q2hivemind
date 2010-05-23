/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MAPPING_GRID_H
#define HM_MAPPING_GRID_H

#include "object.h"
#include "timing.h"
#include "kdtree++/kdtree.hpp"
#include "mapping/items.h"

#include <boost/random/mersenne_twister.hpp>
#include <boost/thread.hpp>

#include <list>
#include <set>

namespace HiveMind {

class Map;
class MapPath;
class Grid;
class GridLink;
class GridNode;

enum {
    DO_NOT_REVISIT_NODE_TIME = 5000
};

/**
 * This represents a waypoint.
 */
class GridWaypoint {
public:
    /**
     * Class constructor.
     *
     * @param location Waypoint location
     */
    GridWaypoint(const Vector3f &location);
    
    /**
     * Returns waypoint coordinates.
     */
    inline Vector3f getLocation() const { return m_location; }
    
    /**
     * Dimension accessor used by the kd-tree.
     *
     * @param n Component index
     */
    inline float operator[](size_t const n) const { return m_location[n]; }
    
    /**
     * Equality operator.
     */
    inline bool operator==(const GridWaypoint &other) const { return m_location == other.m_location; }
    
    /**
     * Less than operator.
     */
    inline bool operator<(const GridWaypoint &other) const
    {
      return m_location[0] < other.m_location[0] || (
               m_location[0] == other.m_location[0] && (
                 m_location[1] < other.m_location[1] || (
                   m_location[1] == m_location[1] && (
                     m_location[2] < other.m_location[2]
                   )
                 )
               )
             );
    }
    
    /**
     * Hash function.
     */
    friend std::size_t hash_value(const GridWaypoint &p)
    {
      std::size_t seed = 0;
      boost::hash_combine(seed, p[0]);
      boost::hash_combine(seed, p[1]);
      boost::hash_combine(seed, p[2]);
      return seed;
    }
private:
    // Waypoint location
    Vector3f m_location;
};

// Helper typedefs
typedef boost::unordered_map<GridNode*, GridLink*> GridLinkMap;
typedef std::set<GridWaypoint> GridWaypointSet;
typedef std::set<Item> ItemSet;

/**
 * This represents a graph node that can contain multiple
 * waypoints.
 */
class GridNode {
friend class Grid;
public:
    /**
     * Possible mediums for a grid node.
     */
    enum Medium {
      Unknown,
      Ground,
      Air,
      Water
    };
    
    /**
     * Possible type of this grid node.
     */
    enum Type {
      Normal,
      SpawnPoint,
      Item
    };
    
    /**
     * Class constructor.
     */
    GridNode(Grid *grid);
    
    /**
     * Class destructor.
     */
    ~GridNode();
    
    /**
     * Adds a waypoint to this grid node.
     */
    void addWaypoint(const GridWaypoint &p);
    
    /**
     * Returns the central location of this grid node. This is
     * always the first waypoint.
     */
    inline Vector3f getLocation() const { return m_location; } 
    
    /**
     * Returns the link map for this node.
     */
    inline GridLinkMap &links() { return m_links; }
    
    /**
     * Returns the waypoint set for this node.
     */
    inline GridWaypointSet &waypoints() { return m_waypoints; }
    
    /**
     * Adds a new link to some other node.
     *
     * @param other Other grid node
     * @param weight Initial link rank
     * @param reinforce Should existing link be reinforced (for duplicates)
     */
    void addLink(GridNode *other, float weight = 1.0, bool reinforce = true);
    
    /**
     * Heuristic function that returns a distance estimate between
     * this node and goal node.
     *
     * @param goal Goal node instance
     */
    inline float heuristic(const GridNode *goal) const { return (getLocation() - goal->getLocation()).norm(); }
    
    /**
     * Returns the medium of this grid node.
     */
    inline Medium getMedium() const { return m_medium; }
    
    /**
     * Sets this grid node's medium.
     *
     * @param medium New medium
     */
    inline void setMedium(Medium medium) { m_medium = medium; }
    
    /**
     * Returns the type of this grid node.
     */
    inline Type getType() const { return m_type; }
    
    /**
     * Sets this grid node's type.
     *
     * @param type New type
     */
    inline void setType(Type type) { m_type = type; }

    /**
     * Returns the time when this GridNode was last visited.
     */
    inline timestamp_t getLastVisit() { return m_lastVisit; }

    /**
     * Sets this grid node's last visit time to now.
     */
    inline void updateLastVisit() { m_lastVisit = Timing::getCurrentTimestamp(); }
    
    /**
     * Adds a new item to this node.
     */
    inline void addItem(const HiveMind::Item &item) { m_items.insert(item); }
    
    /**
     * Returns item list for this node.
     */
    inline ItemSet &items() { return m_items; }
    
    /**
     * Returns true if this node is linked with the rest of the map.
     */
    inline bool isLinked() const { return m_linked; }
private:
    // Grid instance
    Grid *m_grid;
    
    // Node attributes
    Vector3f m_location;
    GridWaypointSet m_waypoints;
    GridLinkMap m_links;
    Medium m_medium;
    Type m_type;
    timestamp_t m_lastVisit;
    bool m_linked;
    
    // Item registry for this node
    ItemSet m_items;
};

/**
 * This represents a link between two grid nodes.
 */
class GridLink {
public:
    /**
     * Class constructor.
     *
     * @param node Destination grid node
     * @param weight Initial link rank
     */
    GridLink(GridNode *node, float weight);
    
    /**
     * Returns the destination grid node.
     */
    inline GridNode *getNode() const { return m_node; }
    
    /**
     * Returns the link's rank.
     */
    inline float getRank() const { return m_rank; }
    
    /**
     * Reinforces this link as a nice traversable link.
     */
    inline void reinforce(float weight = 1.0) { m_rank += weight; }
private:
    GridNode *m_node;
    float m_rank;
};

// Grid KD tree and waypoint map
typedef KDTree::KDTree<3, GridWaypoint, std::pointer_to_binary_function<GridWaypoint,size_t,float> > GridTree;
typedef boost::unordered_map<GridWaypoint, int> GridWaypointIndexMap;

/**
 * A path through the grid.
 */
class GridPath {
public:
    /**
     * Class constructor.
     */
    GridPath();
    
    /**
     * Clears the grid path.
     */
    void clear();
    
    /**
     * Returns true when destination has been reached.
     */
    inline bool isDestinationReached() const { return m_destinationReached; }
    
    /**
     * Adds a new node at the end of this path.
     *
     * @param node A valid GridNode instance
     */
    void add(GridNode *node);
    
    /**
     * Skips the current node.
     */
    void skip();
    
    /**
     * Returns the GridNode that is our next destination point.
     */
    inline GridNode *getCurrent() const { return m_path.at(m_currentNode); }
    
    /**
     * Checks if we have visited any control points.
     *
     * @param point Our current origin
     * @return True if a control point has been reached, false otherwise
     */
    bool visit(const Vector3f &point);
    
    /**
     * Returns the number of hops in this path.
     */
    inline size_t size() const { return m_path.size(); }
    
    /**
     * Performs internal tree datastructure optimsiation.
     */
    void optimiseTree();
private:
    int m_currentNode;
    std::vector<GridNode*> m_path;
    GridTree m_pathTree;
    GridWaypointIndexMap m_waypointMap;
    bool m_destinationReached;
};

/**
 * Grid exporter interface.
 */
class GridExporter {
public:
    /**
     * This method is called on initialization.
     *
     * @param nodes Number of nodes that will be exported
     */
    virtual void open(size_t nodes) = 0;
    
    /**
     * This method is called for every waypoint.
     *
     * @param node Grid node associated with the waypoint
     * @param wp Waypoint
     */
    virtual void exportWaypoint(GridNode *node, const GridWaypoint &wp) = 0;
    
    /**
     * This method is called for every grid node.
     *
     * @param node Grid node
     */
    virtual void exportNode(GridNode *node) = 0;
    
    /**
     * This method is called before links are exported.
     */
    virtual void startLinks() = 0;
    
    /**
     * This method is called for every link.
     *
     * @param node Source grid node
     * @param link Grid link
     */
    virtual void exportLink(GridNode *node, GridLink *link) = 0;
    
    /**
     * This method is called after export has been completed.
     */
    virtual void close() = 0;
};

// Waypoint map
typedef boost::unordered_map<GridWaypoint, GridNode*> GridWaypointNodeMap;

/**
 * Mapping grid.
 */
class Grid : public Object {
friend class GridNode;
public:
    // Cell radius
    enum { cell_radius = 24 };
    
    /**
     * Class constructor.
     *
     * @param map BSP map instance
     */
    Grid(Map *map);
    
    /**
     * Class destructor.
     */
    ~Grid();
    
    /**
     * Clears the grid.
     */
    void clear();
    
    /**
     * Exports the grid.
     *
     * @param exporter Grid exporter
     */
    void exportGrid(GridExporter *exporter);
    
    /**
     * Imports the grid from an external file in internal format.
     *
     * @param filename Import filename
     */ 
    void importGrid(const std::string &filename);
    
    /**
     * Learns map from time ordered waypoint vector.
     *
     * @param locs A series of waypoints
     */
    void learnWaypoints(const std::vector<Vector3f> &locs);
    
    /**
     * Learns a link between two locations.
     *
     * @param locA Location A
     * @param locB Location B
     */
    void learnWaypoints(const Vector3f &locA, const Vector3f &locB);
    
    /**
     * Learns a location that might not be connected with the rest of
     * the graph network.
     *
     * @param loc Location coordinates
     */
    void learnLocation(const Vector3f &loc);
    
    /**
     * Adds a specific grid node as an item node into the registry.
     */
    void learnItem(GridNode *node);
    
    /**
     * Returns the nearest node accoording to some location.
     *
     * @param loc Location coordinates
     * @param radius Search radius
     * @param onlyLinked Should only linked nodes be considered
     * @return Nearest grid node
     */
    GridNode *getNearestNode(const Vector3f &loc, float radius = 100, bool onlyLinked = true);
    
    /**
     * Attempts to find a path through the mapping grid using A* search.
     *
     * @param start Start location coordinates
     * @param end End location coordinates
     * @param path Where to save the path
     * @param full Should a full path be returned
     * @return True when path was found, false otherwise
     */
    bool findPath(const Vector3f &start, const Vector3f &end, GridPath *path, bool full = true);
    
    /**
     * Returns a random path from origin.
     *
     * @param start Start location coordinates
     * @param path Where to save the path
     * @return True when path was found, false otherwise
     */
    bool computeRandomPath(const Vector3f &start, GridPath *path);

    /**
     * Attempts to find a node for a location. If no suitable node
     * exists a new one is created and returned.
     *
     * @param loc Location coordinates
     * @param create Should a new node be created when not found
     * @return A valid GridNode instance or NULL
     */
    GridNode *getNodeByLocation(const Vector3f &loc, bool create = true);
protected:
    /**
     * A helper method to generate a random number.
     */
    int rollDie(int from, int to) const;
    
    /**
     * Returns a node that is linked from start node.
     * 
     * @param start Start GridNode
     * @param visitedNodes Set of nodes that have been visited in this path construction
     * @return GridNode when successive node is found or NULL if successive node cannot be found 
     *         (if there is no link from start node or if all nodes from start node have been visited already = cycle)
     */
    GridNode* pickNextNode(GridNode *start, const std::set<GridNode*> &visitedNodes) const;
private:
    // Static geometry map
    Map *m_map;
    
    // Lookup data structures
    GridTree m_tree;
    GridWaypointNodeMap m_waypointMap;
    boost::unordered_map<Item::Type, GridTree> m_items;
    
    // Random generator
    mutable boost::mt19937 m_gen;
    
    // Mutex
    boost::shared_mutex m_mutex;
};

}

#endif

