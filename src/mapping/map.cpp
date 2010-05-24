/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 * Copyright (C) 1999 by Ben Swarzlander (Q2BotCore)
 */
#include "mapping/map.h"
#include "mapping/bsp.h"
#include "context.h"
#include "logger.h"

#include <fcntl.h>

#include <vector>
#include <set>
#include <queue>
#include <algorithm>

#include <boost/foreach.hpp>

using namespace HiveMind::BSP;

namespace HiveMind {

// Private map attributes
class MapPrivate {
public:
    // Loaded map info
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
    
    // Computed map info
    std::vector<MapFace*> xfaces;
    std::vector<MapLink*> links;
    edgeface_t *edgefaces;
    int edgefriendCount;
    int *edgefriends;
    xedge_t *xedges;
    std::vector<int> sortededges;
};

MapFace::MapFace(int index)
  : m_index(index),
    m_type(0),
    m_origin(0, 0, 0)
{
}

MapLink::MapLink(MapFace *face, const Vector3f &origin)
  : m_cost(1.0),
    m_lastVisited(0),
    m_face(face),
    m_valid(true),
    m_origin(origin)
{
}

void MapLink::updateVisited()
{
  m_lastVisited = Timing::getCurrentTimestamp();
}

void MapLink::applyCost(float cost)
{
  m_cost *= cost;
}

void MapFace::addLink(MapLink *link)
{
  if (link == NULL)
    return;
  
  m_links.push_back(link);
}

Map::Map(Context *context, const std::string &name)
  : m_context(context),
    m_loaded(false),
    m_name(name),
    d(new MapPrivate())
{
  Object::init();
  
  // Convert map name to lower case to avoid possible incosistancies
  std::transform(m_name.begin(), m_name.end(), m_name.begin(), std::ptr_fun(::tolower));

  // Initialize paks
  m_paks.push_back("baseq2/pak0.pak");
  m_paks.push_back("baseq2/pak1.pak");
  m_paks.push_back("baseq2/pak2.pak");
  m_paks.push_back("ctf/pak0.pak");
}

Map::~Map()
{
  // TODO free stuff
}

bool Map::open()
{
  if (m_loaded)
    return true;
  
  // Load the map
  if (!load()) {
    getLogger()->warning("Map loading has failed.");
    return false;
  }
  
  // Link the map
  if (!link()) {
    getLogger()->warning("Map linking has failed.");
    return false;
  }
  
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

MapLink *Map::getLink(int linkId) const
{
  if (linkId < 0 || linkId >= d->links.size())
    return NULL;
  
  return d->links[linkId];
}

// XXX use eigen2
float yawFromVect(vec3_t delta) {
	if(delta[0]==0) {
		if(delta[1]>=0) {
			return M_PI/2;
		} else {
			return 3*M_PI/2;
		}
	} else {
		if(delta[0]>=0) {
			if(delta[1]>=0) {
				return (float)atan(delta[1]/delta[0]);
			} else {
				return 2*M_PI+(float)atan(delta[1]/delta[0]);
			}
		} else {
			return M_PI+(float)atan(delta[1]/delta[0]);
		}
	}
}

// XXX use eigen2
float pitchFromVect(vec3_t delta) {
	float delta2;

	delta2=sqrt(delta[0]*delta[0]+delta[1]*delta[1]);
	if(delta2==0) {
		if(delta[2]>=0) {
			return M_PI/2;
		} else {
			return 3*M_PI/2;
		}
	} else {
		if(delta2>=0) {
			if(delta[2]>=0) {
				return (float)atan(delta[2]/delta2);
			} else {
				return 2*M_PI+(float)atan(delta[2]/delta2);
			}
		} else {
			return M_PI+(float)atan(delta[2]/delta2);
		}
	}
}

// XXX use eigen2
float distFromVect(vec3_t u,vec3_t v) {
	float x,y,z;

	x=v[0]-u[0];
	y=v[1]-u[1];
	z=v[2]-u[2];
	return (float)sqrt(x*x+y*y+z*z);
}

// Edge comparison function
class edge_compare : public std::binary_function<int, int, bool>
{
public:
  edge_compare(MapPrivate *mp, bool key1 = true)
    : m_p(mp),
      m_key1(key1)
  {}
  
  bool operator()(int a, int b) const
  {
    return m_key1 ? m_p->xedges[a].key < m_p->xedges[b].key : m_p->xedges[a].key2 < m_p->xedges[b].key2;
  }
private:
  MapPrivate *m_p;
  bool m_key1;
};

MapLink *Map::createLink(int face, int n, int k)
{
  int faceId = (int) d->edgefaces[n].face[face];
  if (faceId < 0)
    return NULL;
  
  // Create a new map link
  MapLink *link = new MapLink(
    d->xfaces[faceId],
    Vector3f(
      (d->vertices[d->edges[k].v[0]].origin[0] + d->vertices[d->edges[k].v[1]].origin[0]) / 2,
      (d->vertices[d->edges[k].v[0]].origin[1] + d->vertices[d->edges[k].v[1]].origin[1]) / 2,
      (d->vertices[d->edges[k].v[0]].origin[2] + d->vertices[d->edges[k].v[1]].origin[2]) / 2
    )
  );
  
  d->links.push_back(link);
  return link;
}

bool Map::link()
{
  // Initialize structures
  d->edgefriends = (int*) malloc(65536 * sizeof(int));
  d->edgefaces = (edgeface_t*) malloc(d->edgeCount * sizeof(edgeface_t));
  d->xedges = (xedge_t*) malloc(d->edgeCount * sizeof(xedge_t));
  d->sortededges.resize(d->edgeCount);
  
  for (int i = 0; i < d->edgeCount; i++) {
    vec3_t u;
    
    d->edgefaces[i].face[0] = -1;
    d->edgefaces[i].face[1] = -1;
    
    u[0] = d->vertices[d->edges[i].v[1]].origin[0] - d->vertices[d->edges[i].v[0]].origin[0];
    u[1] = d->vertices[d->edges[i].v[1]].origin[1] - d->vertices[d->edges[i].v[0]].origin[1];
    u[2] = d->vertices[d->edges[i].v[1]].origin[2] - d->vertices[d->edges[i].v[0]].origin[2];
    
    d->xedges[i].pitch = (unsigned char) (pitchFromVect(u) * 256.0 / M_PI);
    d->xedges[i].yaw = (unsigned char) (yawFromVect(u) * 256.0 / M_PI);
    d->xedges[i].key = (unsigned short) d->xedges[i].yaw + (((unsigned short) d->xedges[i].pitch) << 8);
    
    switch (d->xedges[i].key) {
      case 0: {
        d->xedges[i].key2 = ((unsigned int) (d->vertices[d->edges[i].v[0]].origin[1] + 32768.0)) & 0xffff + (((unsigned int) (d->vertices[d->edges[i].v[0]].origin[2] + 32768.0)) << 16);
        break;
      }
      
      case 128: {
        d->xedges[i].key2 = ((unsigned int) (d->vertices[d->edges[i].v[0]].origin[0] + 32768.0)) & 0xffff + (((unsigned int) (d->vertices[d->edges[i].v[0]].origin[2] + 32768.0)) << 16);
        break;
      }
      
      case 32896: {
        d->xedges[i].key2 = ((unsigned int) (d->vertices[d->edges[i].v[0]].origin[0] + 32768.0)) & 0xffff + (((unsigned int) (d->vertices[d->edges[i].v[0]].origin[1] + 32768.0)) << 16);
        break;
      }
      
      default: {
        d->xedges[i].key2 = 0;
        break;
      }
    }
    
    d->sortededges[i] = i;
  }
  
  // Sort edges
  edge_compare compare_fun(d);
  std::sort(d->sortededges.begin(), d->sortededges.end(), compare_fun);
  
  // Find friends
  if (!findFriends())
    return false;
  
  // Now come the faces
  for (int i = 0; i < d->faceCount; i++) {
    MapFace *face = new MapFace(d->xfaces.size());
    long type = 0;
    
    if (d->planes[d->faces[i].planenum].normal[2] > 0.7 && d->faces[i].side == 0) {
      type |= 0x00000001;
    } else {
      type &= 0xfffffffe;
    }
    
    if (d->planes[d->faces[i].planenum].normal[2] == 0.0) {
      type |= 0x00000002;
    } else {
      type &= 0xfffffffd;
    }
    
    face->setType(type);
    float f = 1.0 / (float) d->faces[i].numedges;
    for (int j = 0; j < d->faces[i].numedges; j++) {
      int k = d->surfedges[j + d->faces[i].firstedge];
      int v;
      
      if (k > 0) {
        v = d->edges[k].v[0];
      } else {
        k = -k;
        v = d->edges[k].v[1];
      }
      
      // Update face origin
      face->setOrigin(face->getOrigin() + f*Vector3f(
        d->vertices[v].origin[0],
        d->vertices[v].origin[1],
        d->vertices[v].origin[2]
      ));
      
      if (d->edgefaces[k].face[0] == -1) {
        d->edgefaces[k].face[0] = i;
      } else if (d->edgefaces[k].face[1] == -1) {
        d->edgefaces[k].face[1] = i;
      } else {
        getLogger()->warning("More than 2 faces reference the same edge.");
        return false;
      }
    }
    
    d->xfaces.push_back(face);
  }
  
  // Now finally create the links
  for (int m = 0; m < d->modelCount; m++) {
    for (int i = 0; i < d->models[m].numfaces; i++) {
      if (d->xfaces[i]->getType() & 0x00000001) {
        for (int j = 0; j < d->faces[i].numedges; j++) {
          int k = d->surfedges[j + d->faces[i].firstedge];
          if (k < 0) {
            k = -k;
          }
          
          if (d->edgefaces[k].face[0] == i) {
            if (d->edgefaces[k].face[1] != -1) {
              if (d->xfaces[d->edgefaces[k].face[1]]->getType() & 0x00000001) {
                // Create a new link
                d->xfaces[i]->addLink(createLink(1, k, k));
              } else if (d->xfaces[d->edgefaces[k].face[1]]->getType() & 0x00000002) {
                if (!checkWall(d->edgefaces[k].face[1], i, k)) {
                  return false;
                }
              }
            }
          } else if (d->edgefaces[k].face[1] == i) {
            if (d->edgefaces[k].face[0] != -1) {
              if (d->xfaces[d->edgefaces[k].face[0]]->getType() & 0x00000001) {
                // Create a new link
                d->xfaces[i]->addLink(createLink(0, k, k));
              } else if (d->xfaces[d->edgefaces[k].face[0]]->getType() & 0x00000002) {
                if (!checkWall(d->edgefaces[k].face[0], i, k)) {
                  return false;
                }
              }
            }
          } else {
            getLogger()->warning("Neither face links back to original.");
            return false;
          }
          
          for (int l = 0; l < d->xedges[k].numfriends; l++) {
            int n = d->edgefriends[d->xedges[k].firstfriend + l];
            
            if (d->edgefaces[n].face[0] != -1) {
              if (d->xfaces[d->edgefaces[n].face[0]]->getType() & 0x00000001) {
                // Change n to k
                d->xfaces[i]->addLink(createLink(0, n, k));
              } else if (d->xfaces[d->edgefaces[n].face[0]]->getType() & 0x00000002) {
                if (!checkWall(d->edgefaces[n].face[0], i, n)) {
                  return false;
                }
              }
            } else if (d->edgefaces[n].face[1] != -1) {
              if (d->xfaces[d->edgefaces[n].face[1]]->getType() & 0x00000001) {
                // Change n to k
                d->xfaces[i]->addLink(createLink(1, n, k));
              } else if (d->xfaces[d->edgefaces[n].face[1]]->getType() & 0x00000002) {
                if (!checkWall(d->edgefaces[n].face[1], i, n)) {
                  return false;
                }
              }
            }
          }
        }
      }
    }
  }
  
  // We are done linking the map
  getLogger()->info(format("Linked map and produced %d links.") % d->links.size());
  
  return true;
}

bool Map::findFriends()
{
  bool flag = true;
  int first, last, edge, edge2;
  unsigned short key = 65535;
  
  d->edgefriendCount = 0;
  
  for (int i = 0; i < d->edgeCount; i++) {
    flag = true;
    edge = d->sortededges[i];
    d->xedges[edge].firstfriend = d->edgefriendCount;
    d->xedges[edge].numfriends = 0;
    
    if (d->xedges[edge].key != key) {
      key = d->xedges[edge].key;
      
      for (first = 0; d->xedges[d->sortededges[first]].key != key; first++) {
      }
      
      for (last = first; d->xedges[d->sortededges[last]].key == key; last++) {
        if (last == d->edgeCount - 1) {
          break;
        }
      }
      
      if (key == 0 || key == 128 || key == 32896) {
        flag = false;
        
        // Sort by other key
        edge_compare compare_fun(d, false);
        std::sort(d->sortededges.begin() + first, d->sortededges.begin() + last + 1, compare_fun);
        
        if (!findFriends2(first, last)) {
          return false;
        }
        
        i = last - 1;
      }
    }
    
    if (flag) {
      for (int j = first; j < last; j++) {
        edge2 = d->sortededges[j];
        
        if (edge2 != edge) {
          if (edgeOverlap(edge, edge2)) {
            if (colinear(edge, edge2)) {
              d->edgefriends[d->edgefriendCount] = edge2;
              d->xedges[edge].numfriends++;
              d->edgefriendCount++;
            }
          }
        }
        
        if (d->edgefriendCount == 65535) {
          getLogger()->warning("Edge friend table overflow.");
          return false;
        }
      }
    }
  }
  
  return true;
}

bool Map::findFriends2(int start, int end)
{
  int first = 0;
  int last = 0;
  int edge, edge2;
  unsigned short key2 = 0;
  
  for (int i = start; i < end; i++) {
    edge = d->sortededges[i];
    d->xedges[edge].firstfriend = d->edgefriendCount;
    d->xedges[edge].numfriends = 0;
    
    if (d->xedges[edge].key2 != key2) {
      key2 = d->xedges[edge].key2;
      
      for (first = start; d->xedges[d->sortededges[first]].key2 != key2; first++) {
      }
      
      for (last = first; d->xedges[d->sortededges[last]].key2 == key2; last++) {
        if (last == end - 1) {
          break;
        }
      }
    }
    
    for (int j = first; j < last; j++) {
      edge2 = d->sortededges[j];
      
      if (edge2 != edge) {
        if (edgeOverlap(edge, edge2)) {
          if (colinear(edge, edge2)) {
            d->edgefriends[d->edgefriendCount] = edge2;
            d->xedges[edge].numfriends++;
            d->edgefriendCount++;
          }
        }
      }
      
      if (d->edgefriendCount == 65535) {
        getLogger()->warning("Edge friend table overflow.");
        return false;
      }
    }
  }
  
  return true;
}

bool Map::colinear(int m, int n)
{
  vec3_t u;
  unsigned char yaw, pitch;
  unsigned short key;

  u[0] = d->vertices[d->edges[n].v[0]].origin[0] - d->vertices[d->edges[m].v[0]].origin[0];
  u[1] = d->vertices[d->edges[n].v[0]].origin[1] - d->vertices[d->edges[m].v[0]].origin[1];
  u[2] = d->vertices[d->edges[n].v[0]].origin[2] - d->vertices[d->edges[m].v[0]].origin[2];
  
  pitch = (unsigned char) (pitchFromVect(u) * 256.0/M_PI);
  yaw = (unsigned char) (yawFromVect(u) * 256.0/M_PI);
  key = yaw + (pitch << 8);
  return (key == d->xedges[m].key && key == d->xedges[n].key);
}

bool Map::edgeOverlap(int n, int m)
{
  bool p,q;
  float x1, x2, x3, x4, y1, y2, y3, y4, temp;

  x1 = d->vertices[d->edges[n].v[0]].origin[0];
  x2 = d->vertices[d->edges[n].v[1]].origin[0];
  x3=  d->vertices[d->edges[m].v[0]].origin[0];
  x4 = d->vertices[d->edges[m].v[1]].origin[0];
  y1 = d->vertices[d->edges[n].v[0]].origin[1];
  y2 = d->vertices[d->edges[n].v[1]].origin[1];
  y3 = d->vertices[d->edges[m].v[0]].origin[1];
  y4 = d->vertices[d->edges[m].v[1]].origin[1];

  if (x1 > x2) { temp = x1; x1 = x2; x2 = temp; }
  if (x3 > x4) { temp = x3; x3 = x4; x4 = temp; }
  if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; }
  if (y3 > y4) { temp = y3; y3 = y4; y4 = temp; }

  p = ((x1<=x3 && x3<x2) || (x3<=x1 && x1<x4) || (x1==x2 && (x1==x3 || x1==x4)) || (x3==x4 && (x3==x1 || x3==x2)));
  q = ((y1<=y3 && y3<y2) || (y3<=y1 && y1<y4) || (y1==y2 && (y1==y3 || y1==y4)) || (y3==y4 && (y3==y1 || y3==y2)));

  return (p && q);
}

float Map::heightBetween(int n, int m)
{
  float z1, z2, z3, z4;

  z1 = d->vertices[d->edges[n].v[0]].origin[2];
  z2 = d->vertices[d->edges[n].v[1]].origin[2];
  z3 = d->vertices[d->edges[m].v[0]].origin[2];
  z4 = d->vertices[d->edges[m].v[1]].origin[2];
  
  return ((z3 + z4 - z1 - z2) / 2);
}

bool Map::checkWall(int wall, int face, int edge)
{
  for (int j = 0; j < d->faces[wall].numedges; j++) {
    int k = d->surfedges[j + d->faces[wall].firstedge];
    if (k < 0) {
      k = -k;
    }
    
    float height = heightBetween(edge, k);
    if (edgeOverlap(edge, k) && height <= 24.0) {
      if (d->edgefaces[k].face[0] == wall) {
        if (d->edgefaces[k].face[1] != -1) {
          if (d->xfaces[d->edgefaces[k].face[1]]->getType() & 0x00000001 && d->edgefaces[k].face[1] != face) {
            // Change k to edge
            d->xfaces[face]->addLink(createLink(1, k, edge));
          }
        }
      } else if (d->edgefaces[k].face[1] == wall) {
        if (d->edgefaces[k].face[0] != -1) {
          if (d->xfaces[d->edgefaces[k].face[0]]->getType() & 0x00000001 && d->edgefaces[k].face[0] != face) {
            // Change k to edge
            d->xfaces[face]->addLink(createLink(0, k, edge));
          }
        }
      } else {
        getLogger()->warning("Neither face links back to original.");
        return false;
      }
    }
    
    for (int l = 0; l < d->xedges[k].numfriends; l++) {
      int n = d->edgefriends[d->xedges[k].firstfriend + l];
      height = heightBetween(edge, n);
      
      // Change k to n
      if (edgeOverlap(edge, n) && height <= 24.0) {
        if (d->edgefaces[n].face[0] != -1) {
          if (d->xfaces[d->edgefaces[n].face[0]]->getType() & 0x00000001) {
            // Change n to edge
            d->xfaces[face]->addLink(createLink(0, n, edge));
          }
        } else if (d->edgefaces[n].face[1] != -1) {
          if (d->xfaces[d->edgefaces[n].face[1]]->getType() & 0x00000001) {
            // Change k to edge
            d->xfaces[face]->addLink(createLink(1, k, edge));
          }
        }
      }
    }
  }
  
  return true;
}

int Map::findLeafId(const Vector3f &pos) const
{
  // Start at the map root node
  int nextNode = d->models[0].rootnode;
  
  while (nextNode >= 0) {
    plane_t *plane = &(d->planes[d->nodes[nextNode].planenum]);
    
    // Decide where to go next in the BSP tree
    if (plane->type == 0) {
      // X plane
      if ((pos[0] * plane->normal[0]) < plane->dist) {
        nextNode = d->nodes[nextNode].back;
      } else {
        nextNode = d->nodes[nextNode].front;
      }
    } else if (plane->type == 1) {
      // Y plane
      if ((pos[1] * plane->normal[1]) < plane->dist) {
        nextNode = d->nodes[nextNode].back;
      } else {
        nextNode = d->nodes[nextNode].front;
      }
    } else if (plane->type == 2) {
      // Z plane
      if ((pos[2] * plane->normal[2]) < plane->dist) {
        nextNode = d->nodes[nextNode].back;
      } else {
        nextNode = d->nodes[nextNode].front;
      }
    } else {
      // ? plane
      if ((pos[0] * plane->normal[0] + pos[1] * plane->normal[1] + pos[2] * plane->normal[2]) < plane->dist) {
        nextNode = d->nodes[nextNode].back;
      } else {
        nextNode = d->nodes[nextNode].front;
      }
    }
  }
  
  // This wierd return is here because leafs are stored with their
  // negative indices + 1
  return -1 - nextNode;
}
    
int Map::findFaceId(const Vector3f &pos) const
{
  int leaf = findLeafId(pos);
  int firstFace = d->leafs[leaf].firstleafface;
  int lastFace = d->leafs[leaf].numleaffaces + firstFace;
  int possible = 0;
  int hits;
  int v0, v1;
  float dx, dy, t;
  
  for (int i = firstFace; i < lastFace; i++) {
    int face = d->leaffaces[i];
    
    if (d->xfaces[face]->getType() & 0x00000001) {
      int firstEdge = d->faces[face].firstedge;
      int lastEdge = d->faces[face].numedges + firstEdge;
      hits = 0;
      
      for (int j = firstEdge; j < lastEdge; j++) {
        int edge = d->surfedges[j];
        
        if (edge > 0) {
          v0 = d->edges[edge].v[0];
          v1 = d->edges[edge].v[1];
        } else {
          edge = -edge;
          v0 = d->edges[edge].v[1];
          v1 = d->edges[edge].v[0];
        }
        
        dx = d->vertices[v1].origin[0] - d->vertices[v0].origin[0];
        dy = d->vertices[v1].origin[1] - d->vertices[v0].origin[1];
        if (dy == 0) {
          if (pos[1] == d->vertices[v0].origin[1]) {
            if (pos[0] >= d->vertices[v0].origin[0] && pos[0] <= d->vertices[v1].origin[0]) {
              return face;
            }
            
            if (pos[0] >= d->vertices[v1].origin[0] && pos[0] <= d->vertices[v0].origin[0]) {
              return face;
            }
          }
        } else {
          t = (pos[1] - d->vertices[v0].origin[1]) / dy;
          if (t < 1 && t >= 0) {
            if ((t*dx + d->vertices[v0].origin[0]) == pos[0]) {
              return face;
            } else if ((t*dx + d->vertices[v0].origin[0]) >= pos[0]) {
              hits++;
            }
          }
        }
      }
      
      if (hits == 1) {
        return face;
      }
    }
  }
  
  return -1;
}

bool Map::findPath(const Vector3f &start, const Vector3f &end, MapPath *path, bool full) const
{
  // Clear previous path
  path->points.clear();
  path->links.clear();
  
  // Sanity check start position
  if (findLeafId(start) == 0) {
    getLogger()->warning("Start position is outside map!");
    return false;
  }
  
  // Sanity check end position
  if (findLeafId(end) == 0) {
    getLogger()->warning("End position is outside map!");
    return false;
  }
  
  int startFaceId = findFaceId(start);
  int endFaceId = findFaceId(end);
  if (startFaceId == -1 || endFaceId == -1) {
    return false;
  }
  
  MapFace *startFace = d->xfaces[startFaceId];
  MapFace *endFace = d->xfaces[endFaceId];
  
  // Convenience data type for comparisons
  typedef std::pair<float, MapFace*> CostMapFace;
  typedef std::pair<MapFace*, MapLink*> FaceLink;
  
  boost::unordered_map<MapFace*, float> costF, costG;
  std::priority_queue<CostMapFace, std::vector<CostMapFace>, std::greater<CostMapFace> > open;
  boost::unordered_map<MapFace*, bool> openMap;
  boost::unordered_map<MapFace*, bool> closed;
  boost::unordered_map<MapFace*, FaceLink> reversePath;
  bool found = false;
  
  // Initialize A* search
  costG[startFace] = 0;
  costF[startFace] = startFace->heuristic(endFace);
  open.push(CostMapFace(costF[startFace], startFace));
  openMap[startFace] = true;
  
  while (!open.empty()) {
    CostMapFace cmf = open.top();
    MapFace *face = cmf.second;
    open.pop();
    openMap.erase(face);
    
    // Check goal condition
    if (face == endFace) {
      found = true;
      if (!full)
        return true;
      break;
    }
    
    closed[face] = true;
    
    // Check all links
    BOOST_FOREACH(MapLink *link, face->links()) {
      MapFace *neigh = link->getFace();
      if (closed.find(neigh) != closed.end() || !link->isValid())
        continue;
      
      bool better = false;
      float score = costG[face] + link->getCost() * (face->getOrigin() - neigh->getOrigin()).norm();
      if (openMap.find(neigh) == openMap.end()) {
        better = true;
      } else if (score < costG[neigh]) {
        better = true;
      }
      
      if (better) {
        reversePath[neigh] = FaceLink(face, link);
        costG[neigh] = score;
        costF[neigh] = score + neigh->heuristic(endFace);
        open.push(CostMapFace(costF[neigh], neigh));
        openMap[neigh] = true;
      }
    }
  }
  
  // When a path has been found, reconstruct it
  if (found) {
    Vector3f playerHeight(0, 0, 24.);
    MapFace *face = endFace;
    path->points.push_back(end + playerHeight);
    
    while (face != startFace) {
      path->points.push_back(face->getOrigin() + playerHeight);
      
      if (reversePath.find(face) != reversePath.end()) {
        FaceLink fl = reversePath[face];
        path->points.push_back(fl.second->getOrigin() + playerHeight);
        path->links.push_back(fl.second);
        face = fl.first;
      } else {
        return false;
      }
    }
    
    // Insert origin face
    path->points.push_back(startFace->getOrigin() + playerHeight);
    
    // Reverse everything
    std::reverse(path->points.begin(), path->points.end());
    std::reverse(path->links.begin(), path->links.end());
    
    path->length = path->points.size();
    return true;
  }
  
  return false;
}

bool Map::intersectLeaf(int leaf, int mask) const
{
  if (leaf == 0) {
    getLogger()->error("Error in BSP leaf intersection, leaf is zero!");
    return true;
  }
  
  return d->leafs[leaf].contents & mask;
}
    
float Map::intersectTree(const Vector3f &start, const Vector3f &end, int node, float min, float max, int mask) const
{
  int planeId = d->nodes[node].planenum;
  Vector3f t = end - start;
  Vector3f n(
    d->planes[planeId].normal[0],
    d->planes[planeId].normal[1],
    d->planes[planeId].normal[2]
  );
  float pdist = d->planes[planeId].dist;
  float tmp; 
  int farNode, nearNode;
  
  int pType = d->planes[planeId].type;
  switch (pType) {
    case 0:
    case 1:
    case 2: {
      // X, Y or Z planes
      if (t[pType] != 0) {
        tmp = (n[pType] * pdist - start[pType]) / t[pType];
      } else {
        tmp = 999999;
      }
      
      if ((start[pType] * n[pType]) < pdist) {
        nearNode = d->nodes[node].back;
        farNode = d->nodes[node].front;
      } else {
        nearNode = d->nodes[node].front;
        farNode = d->nodes[node].back;
      }
      break;
    }
    
    default: {
      // ? plane
      float l = n.dot(t);
      if (l != 0) {
        tmp = (pdist - start.dot(n)) / l;
      } else {
        tmp = 999999;
      }
      
      if (start.dot(n) < pdist) {
        nearNode = d->nodes[node].back;
        farNode = d->nodes[node].front;
      } else {
        nearNode = d->nodes[node].front;
        farNode = d->nodes[node].back;
      }
      break;
    }
  }
  
  if (tmp >= max || tmp <= 0) {
    // Recurse into near node
    if (nearNode < 0) {
      // Leaf
      if (intersectLeaf(-1 - nearNode, mask))
        return min;
      else
        return 1.0;
    } else {
      // Recurse into near node
      return intersectTree(start, end, nearNode, min, max, mask);
    }
  } else if (tmp <= min) {
    // Recurse into far node
    if (farNode < 0) {
      // Leaf
      if (intersectLeaf(-1 - farNode, mask))
        return min;
      else
        return 1.0;
    } else {
      // Recurse into far node
      return intersectTree(start, end, farNode, min, max, mask);
    }
  } else {
    float test;
    if (nearNode < 0) {
      // Leaf
      if (intersectLeaf(-1 - nearNode, mask))
        test = min;
      else
        test = 1.0;
    } else {
      // Tree
      test = intersectTree(start, end, nearNode, min, tmp, mask);
    }
    
    if (test < 1.0) {
      return test;
    } else {
      // Try far
      if (farNode < 0) {
        // Leaf
        if (intersectLeaf(-1 - farNode, mask))
          return tmp;
        else
          return 1.0;
      } else {
        // Tree
        return intersectTree(start, end, farNode, tmp, max, mask);
      }
    }
  }
}

int Map::pointContents(const Vector3f &point)
{
  int num = d->models[0].rootnode;
  float distance;
  node_t node;
  plane_t plane;
  
  while (num >= 0) {
    node = d->nodes[num];
    plane = d->planes[node.planenum];
    
    if (plane.type < 3)
      distance = point[plane.type] - plane.dist;
    else
      distance = Vector3f(plane.normal).dot(point) - plane.dist;
    
    if (d < 0)
      num = node.back;
    else
      num = node.front;
  }
  
  return d->leafs[-1 - num].contents;
}

float Map::rayTest(const Vector3f &start, const Vector3f &end, int mask)
{
  if (!mask)
    return 1.0;
  
  return intersectTree(start, end, d->models[0].rootnode, 0.0, 1.0, mask);
}

}

