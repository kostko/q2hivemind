/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mapping/exporters.h"

namespace HiveMind {

InternalGridExporter::InternalGridExporter(const std::string &filename)
  : m_out(filename.c_str())
{
}

void InternalGridExporter::open(size_t nodes)
{
  m_lastNodeId = 1;
}

void InternalGridExporter::exportWaypoint(GridNode *node, const GridWaypoint &wp)
{
  Vector3f p = wp.getLocation();
  m_out << "WAYPOINT " << m_nodeIds[node] << " ";
  m_out << p[0] << " " << p[1] << " " << p[2] << std::endl;
}

void InternalGridExporter::exportNode(GridNode *node)
{
  Vector3f p = node->getLocation();
  m_nodeIds[node] = m_lastNodeId;
  m_out << "NODE " << m_lastNodeId << " ";
  m_out << p[0] << " " << p[1] << " " << p[2] << std::endl;
  m_lastNodeId++; 
}

void InternalGridExporter::startLinks()
{
}

void InternalGridExporter::exportLink(GridNode *node, GridLink *link)
{
  int startNode = m_nodeIds[node];
  int endNode = m_nodeIds[link->getNode()];
  m_out << "LINK " << startNode << " " << endNode << " " << link->getRank() << std::endl;
}

void InternalGridExporter::close()
{
  m_out.close();
  m_nodeIds.clear();
}

PajekGridExporter::PajekGridExporter(const std::string &filename)
  : m_out((filename + ".net").c_str()),
    m_outC((filename + ".vec").c_str())
{
}

void PajekGridExporter::open(size_t nodes)
{
  m_lastNodeId = 1;
  m_out << "*vertices " << nodes << std::endl;
  m_outC << "*vertices " << nodes << std::endl;
}

void PajekGridExporter::exportWaypoint(GridNode *node, const GridWaypoint &wp)
{
  // We do not export waypoints to pajek format
}

void PajekGridExporter::exportNode(GridNode *node)
{
  Vector3f p = node->getLocation();
  m_nodeIds[node] = m_lastNodeId;
  m_out << m_lastNodeId << " \"\"" << std::endl;
  m_outC << p[0] << " " << p[1] << " " << p[2] << std::endl;
  m_lastNodeId++; 
}

void PajekGridExporter::startLinks()
{
  m_out << "*arcs" << std::endl;
}

void PajekGridExporter::exportLink(GridNode *node, GridLink *link)
{
  int startNode = m_nodeIds[node];
  int endNode = m_nodeIds[link->getNode()];
  m_out << startNode << " " << endNode << std::endl;
}

void PajekGridExporter::close()
{
  m_out.close();
  m_outC.close();
  m_nodeIds.clear();
}

}


