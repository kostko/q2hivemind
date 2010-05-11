/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MAPPING_MAP_H
#define HM_MAPPING_MAP_H

#include "object.h"
#include "timing.h"

#include <list>
#include <vector>

namespace HiveMind {

class Context;
class MapPrivate;
class MapLink;

// A list of map links
typedef std::list<MapLink*> LinkList;

/**
 * Holds additional face information used for building a path
 * finding graph.
 */
class MapFace {
public:
    /**
     * Class constructor.
     */
    MapFace();
    
    /**
     * Adds an outgoing link to this face.
     *
     * @param link A valid link instance
     */
    void addLink(MapLink *link);
    
    /**
     * Returns the list of links going out of this face.
     */
    inline LinkList &links() { return m_links; }
    
    /**
     * Sets face type.
     *
     * @param type Type flags
     */
    inline void setType(long type) { m_type = type; }
    
    /**
     * Returns this face's type.
     */
    inline long getType() const { return m_type; }
    
    /**
     * Returns this face's origin coordinates.
     */
    inline Vector3f getOrigin() const { return m_origin; }
    
    /**
     * Sets this face's origin coordinates.
     *
     * @param origin Coordinates
     */
    inline void setOrigin(const Vector3f &origin) { m_origin = origin; }
private:
    long m_type;
    Vector3f m_origin;
    LinkList m_links; 
};

/**
 * Map link.
 */
class MapLink {
public:
    /**
     * Class constructor.
     */
    MapLink(int face, const Vector3f &origin);
    
    /**
     * Updates last visited timestamp to current time.
     */
    void updateVisited();
    
    /**
     * Multiplies existing link cost with given value. Bigger costs
     * mean lower probability of using the link.
     *
     * @param cost Cost multiplier
     */
    void applyCost(float cost);
    
    /**
     * Returns the current link cost.
     */
    inline float getCost() const { return m_cost; }
    
    /**
     * Returns the last time the link was visited.
     */
    inline timestamp_t getLastVisited() const { return m_lastVisited; }
    
    /**
     * Returns true if this link is valid.
     */
    inline bool isValid() const { return m_valid; }
    
    /**
     * Returns destination face.
     */
    inline int getFace() const { return m_face; }
    
    /**
     * Returns link origin coordinates. These are interpolated between the
     * two faces.
     */
    inline Vector3f getOrigin() const { return m_origin; }
    
    /**
     * Invalidates this link.
     */
    inline void invalidate() { m_valid = false; }
private:
    int m_face;
    bool m_valid;
    Vector3f m_origin;

    // Link properties
    timestamp_t m_lastVisited;
    float m_cost;
};

/**
 * A path through the map.
 */
struct MapPath {
    // Number of valid points
    int length;
    
    // Point coordinates and link identifiers
    std::vector<Vector3f> points;
    std::vector<MapLink*> links;
};

/**
 * A complete Quake 2 BSP map implementation.
 */
class Map : public Object {
public:
    /**
     * Valid brush types taken from BSP spec.
     */
    enum BrushContents {
      Solid = 1,
      Window = 2,
      Aux = 4,
      Lava = 8,
      Slime = 16,
      Water = 32,
      Mist = 64
    };
    
    /**
     * Class constructor.
     *
     * @param context Hivemind context
     * @param name Map name
     */
    Map(Context *context, const std::string &name);
    
    /**
     * Class destructor.
     */
    virtual ~Map();
    
    /**
     * Opens the map, loading and linking it.
     */
    bool open();
    
    /**
     * Performs a path finding algorithm through the map, finding a
     * potential path. This still needs to be vastly improved.
     *
     * @param start Origin (usually the bot's position)
     * @param end Destination coordinates
     * @param path Where to save the path
     * @param full Should a full path be returned or not
     * @return True when the path was found
     */
    bool findPath(const Vector3f &start, const Vector3f &end, MapPath *path, bool full = true);
    
    /**
     * Generates a random path from a specific location. This still
     * needs to be vastly improved.
     *
     * @param start Origin (usually the bot's position)
     * @param path Where to save the path
     * @return True when the path was found
     */
    bool randomPath(const Vector3f &start, MapPath *path); 
    
    /**
     * Casts a ray from start vector to end vector and checks if there
     * are any intersections inbetween. Mask specifies the kind of brush
     * contents that should be treated as solid.
     *
     * @param start Start vector
     * @param end End vector
     * @param mask Brush contents mask
     * @return Fraction of distance where the hit ocurred
     */
    float rayTest(const Vector3f &start, const Vector3f &end, int mask);
    
    /**
     * Returns the specified link object.
     *
     * @param linkId Link identifier
     * @return Associated MapLink instance or NULL when id is invalid
     */
    MapLink *getLink(int linkId) const;
protected:
    /**
     * Loads the map.
     *
     * @return True if load was successful
     */
    bool load();
    
    /**
     * Links the map, creating a graph to be used for path finding.
     *
     * @return True if linking was successful
     */
    bool link();
    
    /**
     * Helper method for linking.
     */
    bool findFriends();
    
    /**
     * Helper method for linking.
     */
    bool findFriends2(int start, int end);
    
    /**
     * Returns true if two edges are colinear.
     *
     * @param m First edge identifier
     * @param n Second edge identifier
     */
    bool colinear(int m, int n);
    
    /**
     * Returns true if two edges overlap.
     *
     * @param m First edge identifier
     * @param n Second edge identifier
     */
    bool edgeOverlap(int n, int m);
    
    /**
     * Computes the height between two edges.
     *
     * @param m First edge identifier
     * @param n Second edge identifier
     */
    float heightBetween(int n, int m);
    
    /**
     * Helper method for linking.
     */
    bool checkWall(int wall, int face, int edge);
    
    /**
     * Finds the leaf the specified position belongs to.
     *
     * @param pos Position vector
     * @return Leaf index (zero if position not in map)
     */
    int findLeafId(const Vector3f &pos) const;
    
    /**
     * Finds the face the specified position belongs to.
     *
     * @param pos Position vector
     * @return Face index or -1 when one cannot be found
     */
    int findFaceId(const Vector3f &pos) const;
    
    /**
     * Performs intersection with a leaf node in BSP tree.
     *
     * @param leaf Leaf node identifier
     * @param mask Contents mask
     * @return True if there is an intersection
     */
    bool intersectLeaf(int leaf, int mask) const;
    
    /**
     * Performs intersection with the BSP tree. Start and end describe
     * the line whose intersection is being checked.
     *
     * @param start Start coordinates
     * @param end End coordinates
     * @param mask Contents mask
     * @return Fractional intersection distance
     */
    float intersectTree(const Vector3f &start, const Vector3f &end, int node, float min, float max, int mask) const;
    
    MapLink *createLink(int face, int n, int k);
private:
    // Context
    Context *m_context;
    
    // Paks
    std::list<std::string> m_paks;
    
    // Map metadata
    bool m_loaded;
    std::string m_name;
    
    // Map attributes
    MapPrivate *d;
};

}

#endif

