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

#include <queue>
#include <fstream>

#include <boost/foreach.hpp>

namespace HiveMind {

GridWaypoint::GridWaypoint(const Vector3f &location)
  : m_location(location)
{
}

GridNode::GridNode()
  : m_medium(Unknown),
    m_type(Normal)
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

Grid::Grid(Map *map)
  : m_map(map),
    m_tree(std::ptr_fun(waypoint_component))
{
  Object::init();
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
  std::pair<Tree::const_iterator, float> found = m_tree.find_nearest(target, cell_radius);
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
  std::pair<Tree::const_iterator, float> found = m_tree.find_nearest(target, radius);
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

bool Grid::findPath(const Vector3f &start, const Vector3f &end, MapPath *path, bool full)
{
  // Clear previous path
  path->points.clear();
  path->links.clear();
  
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
    path->points.push_back(end);
    
    while (node != startNode) {
      path->points.push_back(node->getLocation());
      
      if (reversePath.find(node) != reversePath.end()) {
        NodeLink fl = reversePath[node];
        node = fl.first;
      } else {
        return false;
      }
    }
    
    // Insert origin face
    path->points.push_back(startNode->getLocation());
    
    // Reverse everything
    std::reverse(path->points.begin(), path->points.end());
    //std::reverse(path->links.begin(), path->links.end());
    
    path->length = path->points.size();
    return true;
  }
  
  return found;
}

}


