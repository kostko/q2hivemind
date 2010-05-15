/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MAPPING_EXPORTERS_H
#define HM_MAPPING_EXPORTERS_H

#include "mapping/grid.h"

#include <fstream>

namespace HiveMind {

/**
 * An exporter for internal format.
 */
class InternalGridExporter : public GridExporter {
public:
    /**
     * Class constructor.
     *
     * @param filename Output filename
     */
    InternalGridExporter(const std::string &filename);
    
    /**
     * This method is called on initialization.
     *
     * @param nodes Number of nodes that will be exported
     */
    void open(size_t nodes);
    
    /**
     * This method is called for every waypoint.
     *
     * @param node Grid node associated with the waypoint
     * @param wp Waypoint
     */
    void exportWaypoint(GridNode *node, const GridWaypoint &wp);
    
    /**
     * This method is called for every grid node.
     *
     * @param node Grid node
     */
    void exportNode(GridNode *node);
    
    /**
     * This method is called before links are exported.
     */
    void startLinks();
    
    /**
     * This method is called for every link.
     *
     * @param node Source grid node
     * @param link Grid link
     */
    void exportLink(GridNode *node, GridLink *link);
    
    /**
     * This method is called after export has been completed.
     */
    void close();
private:
    // Output stream
    std::ofstream m_out;
    
    // For maintaining node identifiers during build
    int m_lastNodeId;
    boost::unordered_map<GridNode*, int> m_nodeIds;
};

/**
 * An exporter for Pajek format to visualize the graph.
 */
class PajekGridExporter : public GridExporter {
public:
    /**
     * Class constructor.
     *
     * @param filename Output filename
     */
    PajekGridExporter(const std::string &filename);
    
    /**
     * This method is called on initialization.
     *
     * @param nodes Number of nodes that will be exported
     */
    void open(size_t nodes);
    
    /**
     * This method is called for every waypoint.
     *
     * @param node Grid node associated with the waypoint
     * @param wp Waypoint
     */
    void exportWaypoint(GridNode *node, const GridWaypoint &wp);
    
    /**
     * This method is called for every grid node.
     *
     * @param node Grid node
     */
    void exportNode(GridNode *node);
    
    /**
     * This method is called before links are exported.
     */
    void startLinks();
    
    /**
     * This method is called for every link.
     *
     * @param node Source grid node
     * @param link Grid link
     */
    void exportLink(GridNode *node, GridLink *link);
    
    /**
     * This method is called after export has been completed.
     */
    void close();
private:
    // Output stream
    std::ofstream m_out;
    std::ofstream m_outC;
    
    // For maintaining node identifiers during build
    int m_lastNodeId;
    boost::unordered_map<GridNode*, int> m_nodeIds;
};

}

#endif

