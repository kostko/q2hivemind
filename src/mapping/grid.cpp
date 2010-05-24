/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mapping/grid.h"
#include "mapping/map.h"
#include "logger.h"
#include <ctime>
#include <queue>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

namespace HiveMind {

GridWaypoint::GridWaypoint(const Vector3f &location)
  : m_location(location)
{
}

GridNode::GridNode(Grid *grid)
  : m_grid(grid),
    m_medium(Unknown),
    m_type(Normal),
    m_lastVisit(0),
    m_linked(false)
{
}

GridNode::~GridNode()
{
  // Free all links
  typedef std::pair<GridNode*, GridLink*> NodeLinkPair;
  BOOST_FOREACH(NodeLinkPair p, m_links) {
    delete p.second;
  }
}

void GridNode::addWaypoint(const GridWaypoint &p)
{
  if (m_waypoints.empty())
    m_location = p.getLocation();
  
  m_waypoints.insert(p);
}

void GridNode::addLink(GridNode *other, float weight, bool reinforce)
{
  boost::unique_lock<boost::shared_mutex> g(m_grid->m_mutex);
  
  // Check if we need to change linked status
  if (!m_linked && other->isLinked()) {
    std::list<GridNode*> queue;
    std::set<GridNode*> visited;
    queue.push_back(this);
    
    while (!queue.empty()) {
      GridNode *n = queue.front();
      queue.pop_front();
      
      if (visited.find(n) == visited.end()) {
        visited.insert(n);
        n->m_linked = true;
        
        typedef std::pair<GridNode*, GridLink*> NodeLinkPair;
        BOOST_FOREACH(NodeLinkPair p, n->links()) {
          queue.push_back(p.first);
        }
      }
    }
    
    m_linked = true;
  }
  
  // Check if a link already exists so we don't duplicate it
  if (m_links.find(other) == m_links.end()) {
    m_links[other] = new GridLink(other, weight);
  } else if (reinforce) {
    GridLink *link = m_links[other];
    link->reinforce(weight);
  }
}

void GridNode::evaluateMedium()
{
  Medium medium;
  Vector3f p = getLocation() - Vector3f(0, 0, 50);
  float d = m_grid->getMap()->rayTest(getLocation(), p, Map::Solid);
  if (d == 1.0) {
    medium = Air;
  } else {
    medium = Ground;
  }
  
  int contents = m_grid->getMap()->pointContents(getLocation() + Vector3f(0, 0, 20.0));
  if (contents & Map::Water) {
    medium = Water;
  }
  
  setMedium(medium);
}

GridLink::GridLink(GridNode *node, float weight)
  : m_node(node),
    m_rank(weight)
{
}

inline float waypoint_component(GridWaypoint p, size_t n)
{
  return p[n];
}

GridPath::GridPath()
  : m_currentNode(0),
    m_pathTree(std::ptr_fun(waypoint_component)),
    m_destinationReached(false)
{
}

void GridPath::add(GridNode *node)
{
  GridWaypoint wp(node->getLocation());
  m_path.push_back(node);
  m_pathTree.insert(wp);
  m_waypointMap[wp] = m_path.size() - 1;
}

// A predicate for determining if a point is visible
class path_waypoint_visited {
public:
  path_waypoint_visited(const Vector3f &location, int currentIndex, const GridWaypointIndexMap &map)
    : location(location), currentIndex(currentIndex), map(map)
  {}
  
  bool operator()(const GridWaypoint &p) const
  {
    float n = location[2] - p[2];
    if (n < -16 || n > 64)
      return false;
    
    Vector3f v = p.getLocation();
    v[2] = location[2];
    return (location - v).norm() < 24.0f && map.at(p) >= currentIndex;
  }
private:
  Vector3f location;
  int currentIndex;
  GridWaypointIndexMap map;
};

bool GridPath::visit(const Vector3f &point)
{
  std::pair<GridTree::const_iterator, float> found = m_pathTree.find_nearest_if(
    GridWaypoint(point),
    100.0,
    path_waypoint_visited(point, m_currentNode, m_waypointMap)
  );
  
  if (found.first != m_pathTree.end()) {
    // Found a point, that means it is visited
    GridWaypoint wp = *found.first;
    int pointIndex = m_waypointMap.at(wp);
    
    // Check if we have reached the destination
    if (pointIndex == m_path.size() - 1)
      m_destinationReached = true;
    else
      m_currentNode = pointIndex + 1;
    
    return true;
  }
  
  return false;
}

void GridPath::skip()
{
  if (m_currentNode == m_path.size() - 1)
    m_destinationReached = true;
  else
    m_currentNode++;
}

void GridPath::optimiseTree()
{
  m_pathTree.optimise();
}

void GridPath::clear()
{
  m_path.clear();
  m_pathTree.clear();
  m_waypointMap.clear();
  m_destinationReached = false;
  m_currentNode = 0;
}

Grid::Grid(Map *map)
  : m_map(map),
    m_tree(std::ptr_fun(waypoint_component))
{
  Object::init();
  
  // Seed the random generator
  m_gen.seed(static_cast<unsigned int> (std::time(0)));
}

Grid::~Grid()
{
  // Free all nodes
  typedef std::pair<GridWaypoint, GridNode*> WaypointNodePair;
  BOOST_FOREACH(WaypointNodePair p, m_waypointMap) {
    delete p.second;
  }
}

void Grid::clear()
{
  // Free all nodes
  typedef std::pair<GridWaypoint, GridNode*> WaypointNodePair;
  BOOST_FOREACH(WaypointNodePair p, m_waypointMap) {
    delete p.second;
  }
  
  m_tree.clear();
  m_waypointMap.clear();
}

void Grid::learnWaypoints(const std::vector<Vector3f> &locs)
{
  Vector3f previous;
  bool hasPrevious = false;
  
  BOOST_FOREACH(Vector3f p, locs) {
    if (hasPrevious) {
      learnWaypoints(previous, p);
    }
    
    previous = p;
    hasPrevious = true;
  }
  
  // Optimise the tree as we might have added a lot of new nodes
  m_tree.optimise();
  
  getLogger()->info(format("Learned %d waypoints, currently holding %d grid nodes.") % locs.size() % m_tree.size());
}

void Grid::learnWaypoints(const Vector3f &locA, const Vector3f &locB)
{
  GridNode *a = getNodeByLocation(locA);
  GridNode *b = getNodeByLocation(locB);
  
  // Sanity check so we don't create loops
  if (a == b) {
    return;
  }
  
  // Create forward link (this has been tested)
  a->addLink(b);
  
  // Create reverse link (this has not been tested and passing in this
  // way may not actually be possible, so we use a lower weight)
  b->addLink(a, 0.1);
}

void Grid::learnLocation(const Vector3f &loc)
{
  // Simply request a node by location and if one doesn't yet exist, a new node
  // will be created
  getNodeByLocation(loc);
}

void Grid::learnItem(GridNode *node)
{
  // Register all new items in this node
  BOOST_FOREACH(Item item, node->items()) {
    if (m_items.find(item.getType()) == m_items.end()) {
      m_items[item.getType()] = GridTree(std::ptr_fun(waypoint_component));
    }
    
    GridWaypoint wp(item.getLocation());
    if (m_items[item.getType()].find(wp) == m_items[item.getType()].end())
      m_items[item.getType()].insert(wp);
  }
}

GridNode *Grid::getNodeByLocation(const Vector3f &loc, bool create)
{
  boost::unique_lock<boost::shared_mutex> g(m_mutex);
  GridWaypoint target(loc);
  GridNode *node = NULL;
  std::pair<GridTree::const_iterator, float> found = m_tree.find_nearest(target, cell_radius);
  if (found.first == m_tree.end()) {
    // No existing waypoints found in that location, create a new node
    if (create) {
      node = new GridNode(this);
      node->addWaypoint(target);
      node->evaluateMedium();
      m_tree.insert(target);
      m_waypointMap[target] = node;
    }
  } else {
    // At least one waypoint has been found, lookup associated node
    GridWaypoint wp = *found.first;
    node = m_waypointMap[wp];
    node->addWaypoint(target);
  }
  
  return node;
}

// A search predicate that only selects nodes that are linked
class require_linked_node {
public:
    require_linked_node(const GridWaypointNodeMap &map)
      : map(map)
    {}
    
    bool operator()(const GridWaypoint &p) const
    {
      return map.at(p)->isLinked();
    }
private:
    GridWaypointNodeMap map;
};

GridNode *Grid::getNearestNode(const Vector3f &loc, float radius, bool onlyLinked)
{
  GridWaypoint target(loc);
  GridNode *node = NULL;
  std::pair<GridTree::const_iterator, float> found;
  
  if (onlyLinked)
    found = m_tree.find_nearest_if(target, radius, require_linked_node(m_waypointMap));
  else
    found = m_tree.find_nearest(target, radius);
  
  if (found.first != m_tree.end()) {
    GridWaypoint wp = *found.first;
    node = m_waypointMap[wp];
  }
  
  return node;
}

void Grid::exportGrid(GridExporter *exporter)
{
  typedef std::pair<GridWaypoint, GridNode*> WaypointNodePair;
  typedef std::pair<GridNode*, GridLink*> NodeLinkPair;
  
  exporter->open(m_waypointMap.size());
  
  BOOST_FOREACH(WaypointNodePair p, m_waypointMap) {
    exporter->exportNode(p.second);
    
    BOOST_FOREACH(GridWaypoint wp, p.second->waypoints()) {
      exporter->exportWaypoint(p.second, wp);
    }
  }
  
  exporter->startLinks();
  
  BOOST_FOREACH(WaypointNodePair p, m_waypointMap) {
    BOOST_FOREACH(NodeLinkPair r, p.second->links()) {
      exporter->exportLink(p.second, r.second);
    }
  }
  
  exporter->close();
}

void Grid::importGrid(const std::string &filename)
{
  std::ifstream in(filename.c_str());
  if (!in.is_open()) {
    getLogger()->error("Failed to import the mapping grid.");
    return;
  }
  
  int waypointCount = 0;
  int nodeCount = 0;
  int linkCount = 0;
  boost::unordered_map<int, GridNode*> nodeIds;
  clear();
  
  for (;;) {
    std::string type;
    in >> type;
    if (type == "NODE") {
      // A single GridNode
      int nodeId;
      Vector3f location;
      in >> nodeId;
      in >> location[0];
      in >> location[1];
      in >> location[2];
      
      GridNode *node = new GridNode(this);
      node->addWaypoint(location);
      node->m_linked = true;
      m_tree.insert(location);
      m_waypointMap[location] = node;
      nodeIds[nodeId] = node;
      nodeCount++;
    } else if (type == "WAYPOINT") {
      // A single GridWaypoint
      int nodeId;
      Vector3f location;
      in >> nodeId;
      in >> location[0];
      in >> location[1];
      in >> location[2];
      
      GridNode *node = nodeIds[nodeId];
      node->addWaypoint(location);
      waypointCount++;
    } else if (type == "LINK") {
      // A single GridLink
      int nodeAId, nodeBId;
      float rank;
      in >> nodeAId;
      in >> nodeBId;
      in >> rank;
      
      GridNode *nodeA = nodeIds[nodeAId];
      GridNode *nodeB = nodeIds[nodeBId];
      nodeA->addLink(nodeB, rank);
      linkCount++;
    } else {
      break;
    }
  }
  
  // Optimise the tree
  m_tree.optimise();
  
  // Evaluate media for all nodes
  typedef std::pair<GridWaypoint, GridNode*> WaypointNodePair;
  BOOST_FOREACH(WaypointNodePair p, m_waypointMap) {
    p.second->evaluateMedium();
  }
  
  getLogger()->info(format("Imported %d grid nodes, %d grid links and %d waypoints.") % nodeCount % linkCount % waypointCount);
}

// A search predicate that only selects nodes with specific medium
class require_medium {
public:
    require_medium(const GridWaypointNodeMap &map, GridNode::Medium medium)
      : map(map), medium(medium)
    {}
    
    bool operator()(const GridWaypoint &p) const
    {
      GridNode *node = map.at(p);
      return node->isLinked() && node->getMedium() == medium;
    }
private:
    GridWaypointNodeMap map;
    GridNode::Medium medium;
};


GridNode *Grid::getNodeByMedium(const Vector3f &loc, GridNode::Medium medium, float radius)
{
  GridWaypoint target(loc);
  GridNode *node = NULL;
  std::pair<GridTree::const_iterator, float> found = m_tree.find_nearest_if(
    target,
    radius,
    require_medium(m_waypointMap, medium)
  );
  
  if (found.first != m_tree.end()) {
    GridWaypoint wp = *found.first;
    node = m_waypointMap[wp];
  }
  
  return node;
}

/**
 * Structure for comparing two grid nodes.
 */
struct gridnode_cmp {
  /**
   * Function object. These benefit from inlining.
   *
   * @param a Pointer to GridNode
   * @param b Pointer to GridNode
   * @result True if a was less recently visited than b and false otherwise
   */
  bool operator()(GridNode* a, GridNode* b) const {
    return a->getLastVisit() < b->getLastVisit();
  }
};

GridNode* Grid::pickNextNode(GridNode *start, const std::set<GridNode*> &visitedNodes) const
{
  GridLinkMap links = start->links();
  if (links.size() == 0)
    return NULL;
      
  std::vector<GridNode*> linkNodes;
  
  typedef std::pair<GridNode*, GridLink*> NodeLinkPair;
  BOOST_FOREACH(NodeLinkPair p, links) {
    // Skip links going from the ground into the air
    if (start->isGround() && p.first->isAir())
      continue;

    linkNodes.push_back(p.first);
  }

  // Sort grid nodes,
  std::sort(linkNodes.begin(), linkNodes.end(), gridnode_cmp());

  BOOST_FOREACH(GridNode *node, linkNodes) {
    if (visitedNodes.find(node) == visitedNodes.end())
      return node;
  }  

  // Cycle detected when trying to pick next node in path. We will have to backtrack.
  return NULL;
}

bool Grid::computeRandomPath(const Vector3f &start, GridPath *path)
{
  boost::shared_lock<boost::shared_mutex> g(m_mutex);
  
  // Clear previous path
  path->clear();

  GridNode *startNode = getNearestNode(start); 
  GridNode *node = startNode;
  GridNode *nextNode;
  std::set<GridNode*> visitedNodes;

  if (startNode == NULL) {
    // Start node is not known so we can't navigate from there
    return false;
  }

  int pathSize = rollDie(100, 200);
  std::vector<GridNode*> tmp;
  tmp.push_back(node);

  for (int i = 0; i < pathSize; i++) {
    visitedNodes.insert(node);

    // Resolve cycle
    while ((nextNode = pickNextNode(node, visitedNodes)) == NULL) {
      // Start backtracking; pop the cycle node from path but leave
      // it in visitedNodes, so it doesn't get entered again
      tmp.pop_back();
      
      if (tmp.size() == 0)
        return false;
      
      node = tmp.back();
    }
    
    tmp.push_back(nextNode);
    node = nextNode;
  }
  
  // Populate the path structure  
  BOOST_FOREACH(GridNode *n, tmp) {
    path->add(n);
  }
  
  path->optimiseTree();
  return true;
}

int Grid::rollDie(int from, int to) const
{
  boost::uniform_int<> dist(from, to);
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > die(m_gen, dist);
  return die();
}

bool Grid::findPath(const Vector3f &start, const Vector3f &end, GridPath *path, bool full)
{
  boost::shared_lock<boost::shared_mutex> g(m_mutex);
  
  // Clear previous path
  path->clear();
  
  GridNode *startNode = getNearestNode(start);
  GridNode *endNode = getNearestNode(end);
  if (startNode == NULL || endNode == NULL) {
    // Start or end node are not known so we can't navigate there
    return false;
  }
  
  // Convenience data type for comparisons
  typedef std::pair<float, GridNode*> CostGridNode;
  typedef std::pair<GridNode*, GridLink*> NodeLink;
  
  boost::unordered_map<GridNode*, float> costF, costG;
  std::priority_queue<CostGridNode, std::vector<CostGridNode>, std::greater<CostGridNode> > open;
  boost::unordered_map<GridNode*, bool> openMap;
  boost::unordered_map<GridNode*, bool> closed;
  boost::unordered_map<GridNode*, NodeLink> reversePath;
  bool found = false;
  
  // Initialize A* search
  costG[startNode] = 0;
  costF[startNode] = startNode->heuristic(endNode);
  open.push(CostGridNode(costF[startNode], startNode));
  openMap[startNode] = true;
  
  while (!open.empty()) {
    CostGridNode cgn = open.top();
    GridNode *node = cgn.second;
    open.pop();
    openMap.erase(node);
    
    // Check goal condition
    if (node == endNode) {
      found = true;
      if (!full)
        return true;
      break;
    }
    
    closed[node] = true;
    
    // Check all links
    BOOST_FOREACH(NodeLink p, node->links()) {
      GridNode *neigh = p.first;
      if (closed.find(neigh) != closed.end())
        continue;
      
      // Skip links going from the ground into the air
      if (node->isGround() && neigh->isAir())
        continue;
      
      bool better = false;
      float score = costG[node] + 1.0 * (node->getLocation() - neigh->getLocation()).norm(); // TODO use rank
      if (openMap.find(neigh) == openMap.end()) {
        better = true;
      } else if (score < costG[neigh]) {
        better = true;
      }
      
      if (better) {
        reversePath[neigh] = NodeLink(node, p.second);
        costG[neigh] = score;
        costF[neigh] = score + neigh->heuristic(endNode);
        open.push(CostGridNode(costF[neigh], neigh));
        openMap[neigh] = true;
      }
    }
  }
  
  // When a path has been found, reconstruct it
  if (found) {
    GridNode *node = endNode;
    std::vector<GridNode*> tmp;
    
    while (node != startNode) {
      tmp.push_back(node);
      
      if (reversePath.find(node) != reversePath.end()) {
        NodeLink fl = reversePath[node];
        node = fl.first;
      } else {
        return false;
      }
    }
    
    // Insert origin face
    tmp.push_back(startNode);
    
    // Reverse everything
    BOOST_REVERSE_FOREACH(GridNode *node, tmp) {
      path->add(node);
    }
    path->optimiseTree();
    return true;
  }
  
  return found;
}

}


