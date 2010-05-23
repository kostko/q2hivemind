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

GridNode::GridNode()
  : m_medium(Unknown),
    m_type(Normal),
    m_lastVisit(0)
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
  // Check if a link already exists so we don't duplicate it
  if (m_links.find(other) == m_links.end()) {
    m_links[other] = new GridLink(other, weight);
  } else if (reinforce) {
    GridLink *link = m_links[other];
    link->reinforce(weight);
  }
  
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

GridNode *Grid::getNodeByLocation(const Vector3f &loc, bool create)
{
  GridWaypoint target(loc);
  GridNode *node = NULL;
  std::pair<GridTree::const_iterator, float> found = m_tree.find_nearest(target, cell_radius);
  if (found.first == m_tree.end()) {
    // No existing waypoints found in that location, create a new node
    if (create) {
      node = new GridNode();
      node->addWaypoint(target);
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

GridNode *Grid::getNearestNode(const Vector3f &loc, float radius)
{
  GridWaypoint target(loc);
  GridNode *node = NULL;
  std::pair<GridTree::const_iterator, float> found = m_tree.find_nearest(target, radius);
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
      
      GridNode *node = new GridNode();
      node->addWaypoint(location);
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
  
  getLogger()->info(format("Imported %d grid nodes, %d grid links and %d waypoints.") % nodeCount % linkCount % waypointCount);
}

GridNode* Grid::pickNextNode(GridNode *start, const std::set<GridNode*> &visitedNodes) const
{
  GridLinkMap links = start->links();
  if (links.size() == 0)
    return NULL;
  
  int randomElement = rollDie(0, links.size() - 1);
  int counter = 0;

  // Choose random node
  typedef std::pair<GridNode*, GridLink*> NodeLinkPair;
  BOOST_FOREACH(NodeLinkPair p, links) {
    if (counter == randomElement && visitedNodes.find(p.first) == visitedNodes.end()) {
      if (Timing::getCurrentTimestamp() - p.first->getLastVisit() > DO_NOT_REVISIT_NODE_TIME) {
        return p.first;
      } else {
        getLogger()->warning(format("Would pick node %s, but it was visited %f seconds ago.") % p.first % ((Timing::getCurrentTimestamp() - p.first->getLastVisit())/1000));
        break;
      }
    }
    counter++;
  }

  // If randomly chosen node was already visited, let's just pick the first node that
  // has not been visited (don't care for randomness at this point)
  BOOST_FOREACH(NodeLinkPair p, links) {
    if (visitedNodes.find(p.first) == visitedNodes.end()) {
      if (Timing::getCurrentTimestamp() - p.first->getLastVisit() > DO_NOT_REVISIT_NODE_TIME) {
        return p.first;
      } else {
        getLogger()->warning(format("Would pick node %s, but it was visited %f seconds ago.") % p.first % ((Timing::getCurrentTimestamp() - p.first->getLastVisit())/1000));
      }
    }
  }

  // Cycle detected when trying to pick next node in path. We will have to backtrack.
  return NULL;
}

bool Grid::computeRandomPath(const Vector3f &start, GridPath *path)
{
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
      node = tmp.back();

      if (tmp.size() == 0)
        return false;
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


