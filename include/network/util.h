/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_NETWORK_UTIL_H
#define HM_NETWORK_UTIL_H

namespace HiveMind {

namespace Util {

/**
 * Computes a Quake 2 checksum.
 */
unsigned checksum(unsigned char *buf, int nbytes, int seqnum);

}

}

#endif

