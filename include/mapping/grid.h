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
#include "kdtree++/kdtree.hpp"

#include <list>
#include <set>

namespace HiveMind {

class Map;
class MapPath;
class GridLink;
class GridNode;

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

/**
 * This represents a graph node that can contain multiple
 * waypoints.
 */
class GridNode {
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
      Item,
      Weapon
    };
    
    /**
     * Class constructor.
     */
    GridNode();
    
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
private:
    Vector3f m_location;
    GridWaypointSet m_waypoints;
    GridLinkMap m_links;
    Medium m_medium;
    Type m_type;
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

/**
 * Mapping grid.
 */
class Grid : public Object {
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
     * Returns the nearest node accoording to some location.
     *
     * @param loc Location coordinates
     * @param radius Search radius
     * @return Nearest grid node
     */
    GridNode *getNearestNode(const Vector3f &loc, float radius = 100);
    
    /**
     * Attempts to find a path through the mapping grid using A* search.
     *
     * @param start Start location coordinates
     * @param end End location coordinates
     * @param path Where to save the path
     * @param full Should a full path be returned
     * @return True when path was found, false otherwise
     */
    bool findPath(const Vector3f &start, const Vector3f &end, MapPath *path, bool full = true);
protected:
    /**
     * Attempts to find a node for a location. If no suitable node
     * exists a new one is created and returned.
     *
     * @param loc Location coordinates
     * @param create Should a new node be created when not found
     * @return A valid GridNode instance or NULL
     */
    GridNode *getNodeByLocation(const Vector3f &loc, bool create = true);
private:
    // Static geometry map
    Map *m_map;
    
    // Lookup data structures
    typedef KDTree::KDTree<3, GridWaypoint, std::pointer_to_binary_function<GridWaypoint,size_t,float> > Tree;
    Tree m_tree;
    boost::unordered_map<GridWaypoint, GridNode*> m_waypointMap;
};

}

#endif

