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
    xface_t *xfaces;
    int linkCount;
    link_t *links;
    edgeface_t *edgefaces;
    int edgefriendCount;
    int *edgefriends;
    xedge_t *xedges;
    std::vector<int> sortededges;
    
    // Path finding structures
    // XXX this here is rubbish and should be removed/changed
    int *dist;
    int s_lists[256][256];
    int s_listnum[256];
    int path[256];
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
          
          d->dist = (int*) malloc(d->faceCount * sizeof(int));
          
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

// Comparison function
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

bool Map::link()
{
  // Initialize structures
  d->links = (link_t*) malloc(65536 * sizeof(link_t));
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
  d->xfaces = (xface_t*) malloc(d->faceCount * sizeof(xface_t));
  for (int i = 0; i < d->faceCount; i++) {
    d->xfaces[i].type = 0;
    
    if (d->planes[d->faces[i].planenum].normal[2] > 0.7 && d->faces[i].side == 0) {
      d->xfaces[i].type |= 0x00000001;
    } else {
      d->xfaces[i].type &= 0xfffffffe;
    }
    
    if (d->planes[d->faces[i].planenum].normal[2] == 0.0) {
      d->xfaces[i].type |= 0x00000002;
    } else {
      d->xfaces[i].type &= 0xfffffffd;
    }
    
    d->xfaces[i].origin[0] = 0.0;
    d->xfaces[i].origin[1] = 0.0;
    d->xfaces[i].origin[2] = 0.0;
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
      
      d->xfaces[i].origin[0] += d->vertices[v].origin[0] * f;
      d->xfaces[i].origin[1] += d->vertices[v].origin[1] * f;
      d->xfaces[i].origin[2] += d->vertices[v].origin[2] * f;
      if (d->edgefaces[k].face[0] == -1) {
        d->edgefaces[k].face[0] = i;
      } else if (d->edgefaces[k].face[1] == -1) {
        d->edgefaces[k].face[1] = i;
      } else {
        getLogger()->warning("More than 2 faces reference the same edge.");
        return false;
      }
    }
    
    d->xfaces[i].firstlink = 0;
    d->xfaces[i].numlinks = 0;
  }
  
  // Now finally create the links
  d->linkCount = 0;
  
  for (int m = 0; m < d->modelCount; m++) {
    for (int i = 0; i < d->models[m].numfaces; i++) {
      d->xfaces[i].firstlink = d->linkCount;
      d->xfaces[i].numlinks = 0;
      
      if (d->xfaces[i].type & 0x00000001) {
        for (int j = 0; j < d->faces[i].numedges; j++) {
          int k = d->surfedges[j + d->faces[i].firstedge];
          if (k < 0) {
            k = -k;
          }
          
          if (d->edgefaces[k].face[0] == i) {
            if (d->edgefaces[k].face[1] != -1) {
              if (d->xfaces[d->edgefaces[k].face[1]].type & 0x00000001) {
                d->links[d->linkCount].face = (unsigned short) d->edgefaces[k].face[1];
                d->links[d->linkCount].origin[0] = (d->vertices[d->edges[k].v[0]].origin[0] + d->vertices[d->edges[k].v[1]].origin[0]) / 2;
                d->links[d->linkCount].origin[1] = (d->vertices[d->edges[k].v[0]].origin[1] + d->vertices[d->edges[k].v[1]].origin[1]) / 2;
                d->links[d->linkCount].origin[2] = (d->vertices[d->edges[k].v[0]].origin[2] + d->vertices[d->edges[k].v[1]].origin[2]) / 2;
                d->links[d->linkCount].valid = true;
                d->xfaces[i].numlinks++;
                d->linkCount++;
              } else if (d->xfaces[d->edgefaces[k].face[1]].type & 0x00000002) {
                if (!checkWall(d->edgefaces[k].face[1], i, k)) {
                  return false;
                }
              }
            }
          } else if (d->edgefaces[k].face[1] == i) {
            if (d->edgefaces[k].face[0] != -1) {
              if (d->xfaces[d->edgefaces[k].face[0]].type & 0x00000001) {
                d->links[d->linkCount].face = (unsigned short) d->edgefaces[k].face[0];
                d->links[d->linkCount].origin[0] = (d->vertices[d->edges[k].v[0]].origin[0] + d->vertices[d->edges[k].v[1]].origin[0]) / 2;
                d->links[d->linkCount].origin[1] = (d->vertices[d->edges[k].v[0]].origin[1] + d->vertices[d->edges[k].v[1]].origin[1]) / 2;
                d->links[d->linkCount].origin[2] = (d->vertices[d->edges[k].v[0]].origin[2] + d->vertices[d->edges[k].v[1]].origin[2]) / 2;
                d->links[d->linkCount].valid = true;
                d->xfaces[i].numlinks++;
                d->linkCount++;
              } else if (d->xfaces[d->edgefaces[k].face[0]].type & 0x00000002) {
                if (!checkWall(d->edgefaces[k].face[0], i, k)) {
                  return false;
                }
              }
            }
          } else {
            getLogger()->warning("Neither face links back to original.");
            return false;
          }
          
          if (d->linkCount == 65535) {
            getLogger()->warning("Link table overflow.");
            return false;
          }
          
          for (int l = 0; l < d->xedges[k].numfriends; l++) {
            int n = d->edgefriends[d->xedges[k].firstfriend + l];
            
            if (d->edgefaces[n].face[0] != -1) {
              if (d->xfaces[d->edgefaces[n].face[0]].type & 0x00000001) {
                // Change n to k
                d->links[d->linkCount].face = (unsigned short) d->edgefaces[n].face[0];
                d->links[d->linkCount].origin[0] = (d->vertices[d->edges[k].v[0]].origin[0] + d->vertices[d->edges[k].v[1]].origin[0]) / 2;
                d->links[d->linkCount].origin[1] = (d->vertices[d->edges[k].v[0]].origin[1] + d->vertices[d->edges[k].v[1]].origin[1]) / 2;
                d->links[d->linkCount].origin[2] = (d->vertices[d->edges[k].v[0]].origin[2] + d->vertices[d->edges[k].v[1]].origin[2]) / 2;
                d->links[d->linkCount].valid = true;
                d->xfaces[i].numlinks++;
                d->linkCount++;
              } else if (d->xfaces[d->edgefaces[n].face[0]].type & 0x00000002) {
                if (!checkWall(d->edgefaces[n].face[0], i, n)) {
                  return false;
                }
              }
            } else if (d->edgefaces[n].face[1] != -1) {
              if (d->xfaces[d->edgefaces[n].face[1]].type & 0x00000001) {
                // Change n to k
                d->links[d->linkCount].face = (unsigned short) d->edgefaces[n].face[1];
                d->links[d->linkCount].origin[0] = (d->vertices[d->edges[k].v[0]].origin[0] + d->vertices[d->edges[k].v[1]].origin[0]) / 2;
                d->links[d->linkCount].origin[1] = (d->vertices[d->edges[k].v[0]].origin[1] + d->vertices[d->edges[k].v[1]].origin[1]) / 2;
                d->links[d->linkCount].origin[2] = (d->vertices[d->edges[k].v[0]].origin[2] + d->vertices[d->edges[k].v[1]].origin[2]) / 2;
                d->links[d->linkCount].valid = true;
                d->xfaces[i].numlinks++;
                d->linkCount++;
              } else if (d->xfaces[d->edgefaces[n].face[1]].type & 0x00000002) {
                if (!checkWall(d->edgefaces[n].face[1], i, n)) {
                  return false;
                }
              }
            }
            
            if (d->linkCount == 65535) {
              getLogger()->warning("Link table overflow.");
              return false;
            }
          }
        }
      }
    }
  }
  
  // Disable links with unset faces
  for (int i = 0; i < d->linkCount; i++) {
    if (d->links[i].face >= d->faceCount)
      d->links[i].valid = false;
  }
  
  // We are done linking the map
  getLogger()->info(format("Linked map and produced %d links.") % d->linkCount);
  
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
  int first, last, edge, edge2;
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
        if (d->xfaces[d->edgefaces[k].face[1]].type & 0x00000001 && d->edgefaces[k].face[1] != face) {
          // Change k to edge
          d->links[d->linkCount].face = (unsigned short) d->edgefaces[k].face[1];
          d->links[d->linkCount].origin[0] = (d->vertices[d->edges[edge].v[0]].origin[0] + d->vertices[d->edges[edge].v[1]].origin[0]) / 2;
          d->links[d->linkCount].origin[1] = (d->vertices[d->edges[edge].v[0]].origin[1] + d->vertices[d->edges[edge].v[1]].origin[1]) / 2;
          d->links[d->linkCount].origin[2] = (d->vertices[d->edges[edge].v[0]].origin[2] + d->vertices[d->edges[edge].v[1]].origin[2]) / 2;
          d->links[d->linkCount].valid = true;
          d->xfaces[face].numlinks++;
          d->linkCount++;
        }
      } else if (d->edgefaces[k].face[1] == wall) {
        if (d->xfaces[d->edgefaces[k].face[0]].type & 0x00000001 && d->edgefaces[k].face[0] != face) {
          // Change k to edge
          d->links[d->linkCount].face = (unsigned short) d->edgefaces[k].face[0];
          d->links[d->linkCount].origin[0] = (d->vertices[d->edges[edge].v[0]].origin[0] + d->vertices[d->edges[edge].v[1]].origin[0]) / 2;
          d->links[d->linkCount].origin[1] = (d->vertices[d->edges[edge].v[0]].origin[1] + d->vertices[d->edges[edge].v[1]].origin[1]) / 2;
          d->links[d->linkCount].origin[2] = (d->vertices[d->edges[edge].v[0]].origin[2] + d->vertices[d->edges[edge].v[1]].origin[2]) / 2;
          d->links[d->linkCount].valid = true;
          d->xfaces[face].numlinks++;
          d->linkCount++;
        }
      } else {
        getLogger()->warning("Neither face links back to original.");
        return false;
      }
      
      if (d->linkCount == 65535) {
        getLogger()->warning("Link table overflow.");
        return false;
      }
    }
    
    for (int l = 0; l < d->xedges[k].numfriends; l++) {
      int n = d->edgefriends[d->xedges[k].firstfriend + l];
      height = heightBetween(edge, n);
      
      // Change k to n
      if (edgeOverlap(edge, n) && height <= 24.0) {
        if (d->edgefaces[n].face[0] != -1) {
          if (d->xfaces[d->edgefaces[n].face[0]].type & 0x00000001) {
            // Change n to edge
            d->links[d->linkCount].face = (unsigned short) d->edgefaces[n].face[0];
            d->links[d->linkCount].origin[0] = (d->vertices[d->edges[edge].v[0]].origin[0] + d->vertices[d->edges[edge].v[1]].origin[0]) / 2;
            d->links[d->linkCount].origin[1] = (d->vertices[d->edges[edge].v[0]].origin[1] + d->vertices[d->edges[edge].v[1]].origin[1]) / 2;
            d->links[d->linkCount].origin[2] = (d->vertices[d->edges[edge].v[0]].origin[2] + d->vertices[d->edges[edge].v[1]].origin[2]) / 2;
            d->links[d->linkCount].valid = true;
            d->xfaces[face].numlinks++;
            d->linkCount++;
          }
        } else if (d->edgefaces[n].face[1] != -1) {
          if (d->xfaces[d->edgefaces[n].face[1]].type & 0x00000001) {
            // Change k to edge
            d->links[d->linkCount].face = (unsigned short) d->edgefaces[k].face[1];
            d->links[d->linkCount].origin[0] = (d->vertices[d->edges[edge].v[0]].origin[0] + d->vertices[d->edges[edge].v[1]].origin[0]) / 2;
            d->links[d->linkCount].origin[1] = (d->vertices[d->edges[edge].v[0]].origin[1] + d->vertices[d->edges[edge].v[1]].origin[1]) / 2;
            d->links[d->linkCount].origin[2] = (d->vertices[d->edges[edge].v[0]].origin[2] + d->vertices[d->edges[edge].v[1]].origin[2]) / 2;
            d->links[d->linkCount].valid = true;
            d->xfaces[face].numlinks++;
            d->linkCount++;
          }
        }
      
        if (d->linkCount == 65535) {
          getLogger()->warning("Link table overflow.");
          return false;
        }
      }
    }
  }
  
  return true;
}

void Map::markLinkInvalid(int link)
{
  if (link < 0 || link > 65535)
    return;
  
  d->links[link].valid = false;
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
    
    if (d->xfaces[face].type & 0x00000001) {
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

bool Map::findPath(const Vector3f &start, const Vector3f &end, MapPath *path, bool full)
{
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
  
  int startFace = findFaceId(start);
  int endFace = findFaceId(end);
  if (startFace == -1 || endFace == -1) {
    return false;
  }
  
  // Initialize distances
  for (int i = 0; i < d->faceCount; i++) {
    d->dist[i] = -1;
  }
  
  d->s_lists[0][0] = startFace;
  d->s_listnum[0] = 1;
  if (startFace >= d->faceCount || startFace < 0) {
    getLogger()->error("Start face out of bounds!");
    return false;
  }
  
  d->dist[startFace] = 0;
  bool found = false;
  int depth = 0;
  
  // XXX this search is fucking stupid, we should upgrade it to A* and define
  //     some cool heuristic (euclidian distance should suffice)
  
  while (!found) {
    depth++;
    if (depth >= 250) {
      return false;
    }
    
    d->s_listnum[depth] = 0;
    for (int i = 0; i < d->s_listnum[depth - 1]; i++) {
      int face = d->s_lists[depth - 1][i];
      if (face >= d->faceCount || face < 0) {
        getLogger()->error("Face out of bounds!");
        return false;
      }
      
      for (int j = 0; j < d->xfaces[face].numlinks; j++) {
        int link = d->xfaces[face].firstlink + j;
        if (link >= d->linkCount || link < 0) {
          getLogger()->error("Link out of bounds!");
          return false;
        }
        
        // Skip links marked as invalid
        if (!d->links[link].valid)
          continue;
        
        int face2 = d->links[link].face;
        if (face2 >= d->faceCount || face2 < 0) {
          getLogger()->error("Face out of bounds!");
          return false;
        }
        
        if (face2 == endFace) {
          found = true;
        }
        
        if (d->dist[face2] == -1) {
          d->dist[face2] = depth;
          int index = d->s_listnum[depth];
          if (index >= 256) {
            getLogger()->error(format("Too many faces at depth %d!") % depth);
            return false;
          }
          
          d->s_lists[depth][index] = face2;
          d->s_listnum[depth]++;
        }
      }
    }
  }
  
  path->length = depth*2 + 1;
  if (!full)
    return true;
  
  // Reconstruct the path
  int face2 = endFace;
  int face;
  for (int i = depth - 1; i >= 0; i--) {
    found = false;
    for (int j = 0; !found; j++) {
      face = d->s_lists[i][j];
      for (int k = 0; k < d->xfaces[face].numlinks; k++) {
        int link = d->xfaces[face].firstlink + k;
        if (d->links[link].face == face2) {
          found = true;
          d->path[i] = link;
        }
      }
    }
    
    face2 = face;
  }
  
  // + 24.0 is for player height
  
  path->points[0][0] = d->xfaces[startFace].origin[0];
  path->points[0][1] = d->xfaces[startFace].origin[1];
  path->points[0][2] = d->xfaces[startFace].origin[2] + 24.0;
  
  for (int i = 0; i < depth; i++) {
    path->points[2*i+1][0] = d->links[d->path[i]].origin[0];
    path->points[2*i+1][1] = d->links[d->path[i]].origin[1];
    path->points[2*i+1][2] = d->links[d->path[i]].origin[2] + 24.0;
    
    path->points[2*i+2][0] = d->xfaces[d->links[d->path[i]].face].origin[0];
    path->points[2*i+2][1] = d->xfaces[d->links[d->path[i]].face].origin[1];
    path->points[2*i+2][2] = d->xfaces[d->links[d->path[i]].face].origin[2] + 24.0;
    
    path->links[i] = d->path[i];
  }
  
  return true;
}

bool Map::randomPath(const Vector3f &start, MapPath *path)
{
  // Sanity check start position
  if (findLeafId(start) == 0) {
    getLogger()->warning("Start position is outside map!");
    return false;
  }
  
  int face = findFaceId(start);
  if (face == -1) {
    return false;
  }
  
  // Initialize distances
  for (int i = 0; i < d->faceCount; i++) {
    d->dist[i] = -1;
  }
  
  if (face >= d->faceCount || face < 0) {
    getLogger()->error("Start face out of bounds!");
    return false;
  }
  
  d->dist[face] = 0;
  path->points[0][0] = d->xfaces[face].origin[0];
  path->points[0][1] = d->xfaces[face].origin[1];
  path->points[0][2] = d->xfaces[face].origin[2] + 24.0;
  path->length = 1;
  
  int list[256];
  int list2[256];
  
  for (int i = 0; i < 256; i++) {
    if (face >= d->faceCount || face < 0) {
      getLogger()->error("Face out of bounds!");
      return false;
    }
    
    int numlinks = std::min(256, (int) d->xfaces[face].numlinks);
    for (int j = 0; j < numlinks; j++) {
      list2[j] = d->xfaces[face].firstlink + j;
    }
    
    for (int j = numlinks - 1; j >= 0; j--) {
      int k = rand() % (j + 1);
      list[j] = list2[k];
      list2[k] = list2[j-1];
    }
    
    int j;
    for (j = 0; j < numlinks; j++) {
      int link = list[j];
      if (link >= d->linkCount || link < 0) {
        getLogger()->error("Link out of bounds!");
        return false;
      }
      
      if (!d->links[link].valid)
        continue;
      
      int face2 = d->links[link].face;
      if (face2 >= d->faceCount || face2 < 0) {
        getLogger()->error("Face out of bounds!");
        return false;
      }
      
      if (d->dist[face2] == -1) {
        d->dist[face2] = 0;
        face = face2;
        path->points[2*i+1][0] = d->links[link].origin[0];
        path->points[2*i+1][1] = d->links[link].origin[1];
        path->points[2*i+1][2] = d->links[link].origin[2] + 24.0;
        path->points[2*i+2][0] = d->xfaces[face].origin[0];
        path->points[2*i+2][1] = d->xfaces[face].origin[1];
        path->points[2*i+2][2] = d->xfaces[face].origin[2] + 24.0;
        path->links[i] = link;
        path->length = 2*i + 3;
        break;
      }
    }
    
    if (j == numlinks)
      break;
  }
  
  return true;
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

float Map::rayTest(const Vector3f &start, const Vector3f &end, int mask)
{
  if (!mask)
    return 1.0;
  
  return intersectTree(start, end, d->models[0].rootnode, 0.0, 1.0, mask);
}

}

