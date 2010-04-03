/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "mapping/map.h"
#include "mapping/bsp.h"
#include "context.h"
#include "logger.h"

#include <fcntl.h>

#include <boost/foreach.hpp>
#include <boost/format.hpp>

using boost::format;
using namespace HiveMind::BSP;

namespace HiveMind {

// Private map attributes
class MapPrivate {
public:
    int planeCount;
    plane_t *planes;
    int vertexCount;
    vertex_t *vertices;
    int nodeCount;
    node_t *nodes;
    int faceCount;
    face_t *faces;
    int leaffaceCount;
    unsigned short *leaffaces;
    int leafCount;
    leaf_t *leafs;
    int edgeCount;
    edge_t *edges;
    int surfedgeCount;
    int *surfedges;
    int modelCount;
    model_t *models;
};

Map::Map(Context *context, const std::string &name)
  : m_context(context),
    m_loaded(false),
    m_name(name),
    d(new MapPrivate())
{
  Object::init();
  
  // Initialize paks
  m_paks.push_back("baseq2/pak0.pak");
  m_paks.push_back("baseq2/pak1.pak");
  m_paks.push_back("baseq2/pak2.pak");
  m_paks.push_back("ctf/pak0.pak");
}

Map::~Map()
{
}

bool Map::open()
{
  if (m_loaded)
    return true;
  
  // Load the map
  if (!load())
    return false;
  
  // Link the map
  if (!link())
    return false;
  
  m_loaded = true;
  return true;
}

bool Map::load()
{
  // Attempt to load map from a PAK datafile
  int length = 0;
  BOOST_FOREACH(std::string pak, m_paks) {
    int fh = ::open((m_context->getGameDir() + "/" + pak).c_str(), O_RDONLY);
    if (fh != -1) {
      // Parse PAK header
      pak_header_t pakHeader;
      length = read(fh, &pakHeader, sizeof(pak_header_t));
      if (length == -1) {
        close(fh);
        getLogger()->error(format("Error reading from PAK %s!") % pak);
        return false;
      }
      
      // Load directory entries
      int numDirs = pakHeader.dirsize / 0x40;
      pak_dir_entry_t *pakDirEntries = (pak_dir_entry_t*) malloc(pakHeader.dirsize);
      lseek(fh, pakHeader.diroffset, 0);
      length = read(fh, pakDirEntries, pakHeader.dirsize);
      if (length == -1) {
        free(pakDirEntries);
        close(fh);
        getLogger()->error(format("Error reading from PAK %s!") % pak);
        return false;
      }
      
      // Find our map among the entries in PAK VFS
      for (int i = 0; i < numDirs; i++) {
        if (m_name == pakDirEntries[i].filename) {
          bsp_header_t mapHeader;
          
          lseek(fh, pakDirEntries[i].offset, 0);
          read(fh, &mapHeader, sizeof(bsp_header_t));
          if (mapHeader.ident != IDBSPHEADER || mapHeader.version != BSPVERSION) {
            free(pakDirEntries);
            close(fh);
            getLogger()->error(format("Unrecognized map version %d!") % mapHeader.version);
            return false;
          }
          
          // Load map data
          length = mapHeader.planes.size;
          d->planeCount = length / sizeof(plane_t);
          d->planes = (plane_t*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.planes.offset, 0);
          read(fh, d->planes, length);
          
          length = mapHeader.vertices.size;
          d->vertexCount = length / sizeof(vertex_t);
          d->vertices = (vertex_t*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.vertices.offset, 0);
          read(fh, d->vertices, length);
          
          length = mapHeader.nodes.size;
          d->nodeCount = length / sizeof(node_t);
          d->nodes = (node_t*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.nodes.offset, 0);
          read(fh, d->nodes, length);
          
          length = mapHeader.faces.size;
          d->faceCount = length / sizeof(face_t);
          d->faces = (face_t*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.faces.offset, 0);
          read(fh, d->faces, length);
          
          length = mapHeader.leaffaces.size;
          d->leaffaceCount = length / sizeof(unsigned short);
          d->leaffaces = (unsigned short*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.leaffaces.offset, 0);
          read(fh, d->leaffaces, length);
          
          length = mapHeader.leafs.size;
          d->leafCount = length / sizeof(leaf_t);
          d->leafs = (leaf_t*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.leafs.offset, 0);
          read(fh, d->leafs, length);
          
          length = mapHeader.edges.size;
          d->edgeCount = length / sizeof(edge_t);
          d->edges = (edge_t*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.edges.offset, 0);
          read(fh, d->edges, length);
          
          length = mapHeader.surfedges.size;
          d->surfedgeCount = length / sizeof(int);
          d->surfedges = (int*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.surfedges.offset, 0);
          read(fh, d->surfedges, length);
          
          length = mapHeader.models.size;
          d->modelCount = length / sizeof(model_t);
          d->models = (model_t*) malloc(length);
          lseek(fh, pakDirEntries[i].offset + mapHeader.models.offset, 0);
          read(fh, d->models, length);
          
          // Map loading is done
          getLogger()->info(format("Found and loaded map %s from PAK %s.") % m_name % pak);
          free(pakDirEntries);
          close(fh);
          return true;
        }
      }
      
      close(fh);
    }
  }
  
  getLogger()->warning(format("Map %s cannot be found!") % m_name);
  return false;
}
    
bool Map::link()
{
  return true;
}

}

