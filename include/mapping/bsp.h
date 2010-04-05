/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_MAPPING_BSP_H
#define HM_MAPPING_BSP_H

// Magic constants
#define IDBSPHEADER (('P'<<24)+('S'<<16)+('B'<<8)+'I')
#define IDPAKHEADER (('K'<<24)+('C'<<16)+('A'<<8)+'P')
#define BSPVERSION 38

namespace HiveMind {

namespace BSP {

/**
 * PAK datafile header.
 */
typedef struct {
  long magic;
  long diroffset;
  long dirsize;
} pak_header_t;

/**
 * PAK VFS directory entries.
 */
typedef struct {
  char filename[0x38];
  long offset;
  long size;
} pak_dir_entry_t;

/**
 * BSP directory entry.
 */
typedef struct {
  long offset;
  long size;
} bsp_dir_entry_t;

/**
 * BSP map header.
 */
typedef struct {
  long ident;
  long version;
  bsp_dir_entry_t entities;
  bsp_dir_entry_t planes;
  bsp_dir_entry_t vertices;
  bsp_dir_entry_t vislist;
  bsp_dir_entry_t nodes;
  bsp_dir_entry_t texinfo;
  bsp_dir_entry_t faces;
  bsp_dir_entry_t lightmaps;
  bsp_dir_entry_t leafs;
  bsp_dir_entry_t leaffaces;
  bsp_dir_entry_t leafbrushes;
  bsp_dir_entry_t edges;
  bsp_dir_entry_t surfedges;
  bsp_dir_entry_t models;
  bsp_dir_entry_t brushes;
  bsp_dir_entry_t brushsides;
  bsp_dir_entry_t pop;
  bsp_dir_entry_t areas;
  bsp_dir_entry_t areaportals;
} bsp_header_t;

typedef float vec3_t[3];

typedef struct {
  vec3_t mins, maxs;
  vec3_t origin;
  long rootnode;
  long firstface;
  long numfaces;
} model_t;

typedef struct {
  vec3_t origin;
} vertex_t;

typedef struct {
  vec3_t normal;
  float dist;
  long type;
} plane_t;

typedef struct {
  long planenum;
  long front; // negative numbers are -(leafs+1), not nodes
  long back;
  short mins[3];
  short maxs[3];
  unsigned short firstface;
  unsigned short numfaces;
} node_t;

typedef struct {
  float vecs[2][4]; // [s/t][xyz offset]
  long flags; // miptex flags + overrides
  long value; // light emission, etc
  char texture[32]; // texture name (textures/*.wal)
  long nexttexinfo; // for animations, -1 = end of chain
} texinfo_t;

typedef struct {
  unsigned short v[2];
} edge_t;

typedef struct {
  unsigned short planenum;
  short side;
  long firstedge;
  short numedges;
  short texinfo;
  char styles[4];
  long lightofs;
} face_t;

typedef struct {
  long contents; // OR of all brushes (not needed?)
  short cluster;
  short area;
  short mins[3];
  short maxs[3];
  unsigned short firstleafface;
  unsigned short numleaffaces;
  unsigned short firstleafbrush;
  unsigned short numleafbrushes;
} leaf_t;

typedef struct {
  unsigned short planenum;
  short texinfo;
} brushside_t;

typedef struct {
  long firstside;
  long numsides;
  long contents;
} brush_t;

typedef struct {
  long numclusters;
  long bitofs[8][2];
} vis_t;

typedef struct {
  long portalnum;
  long otherarea;
} areaportal_t;

typedef struct {
  long numareaportals;
  long firstareaportal;
} area_t;

typedef struct {
  long type;
  unsigned short firstlink;
  unsigned short numlinks;
  vec3_t origin;
} xface_t;

typedef struct {
  unsigned char pitch;
  unsigned char yaw;
  unsigned short key;
  unsigned int key2;
  unsigned short firstfriend;
  unsigned short numfriends;
} xedge_t;

typedef struct {
  unsigned short face;
  bool valid;
  char pad;
  vec3_t origin;
} link_t;

typedef struct {
  long face[2];
} edgeface_t;

}

}

#endif

