/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_RL_ENUMVECTOR_H
#define HM_RL_ENUMVECTOR_H

#include <vector>

/**
 * An enumerable vector - its components take only 
 * a finite number of possible values.
 *
 * Concept taken from: http://www.compapp.dcu.ie/~humphrys/Notes/RL/Code/index.html
 */
class EnumVector {
public:
    /**
     * Constructor.
     *
     * @param components components[i] is the number of states that the i-th component can take.
     *                   Note: this also means that the values of component i go from 0 to components[i]-1!
     */ 
     EnumVector(std::vector<int> &components);
     
     /**
      * Destructor.
      */
     ~EnumVector();
     
     /**
      * This vector's ID.
      */
     int id();
     
     /**
      * Makes a vector from the given id.
      */
     void from(int id);
     
     /**
      * Number of permutations.
      */
     int permutations() const { return m_p; }
     
     /**
      * Element access by index.
      */
     int& operator[](int i);
     
     /**
      * Assignment operator.
      */
     EnumVector& operator=(EnumVector &other);
     
private:
    int m_p;                    // Number of possible vectors.
    std::vector<int> m_data;         // Data holder.
    std::vector<int> m_components;   // Element i of m_components represents how many states the i-th component can take.
};

#endif
