/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "rl/enumvector.h"

EnumVector::EnumVector(std::vector<int> &components)
  : m_components(components),
    m_data(components.size(), 0)
{
  // Compute the number of possible permutations
  m_p = 1;
  for (int i = 0; i < m_components.size(); i++)
    m_p = m_p * m_components[i];
}

EnumVector::~EnumVector()
{
  delete &m_components;
  delete &m_data;
}

int EnumVector::id()
{
  int id = 0;
  int p = 1;    // Number of permutations of the sub-vector from n to i
  
  for (int i = m_data.size()-1; i >= 0; i--) {
    id = id + ((*this)[i] * p);
    p = p * m_components[i];
  }
  
  return id;
}

void EnumVector::from(int id)
{
  int p = m_p;
  
  for (int i = 0; i < m_data.size(); i++) {
    p = p / m_components[i];
    (*this)[i] = id / p;
    id = id % p;
  }
}

int& EnumVector::operator[](int i)
{
  return m_data[i];
}

EnumVector& EnumVector::operator=(EnumVector &other)
{
  m_components = other.m_components;
  m_data = other.m_data;
  m_p = other.m_p;
  
  return *this;
}