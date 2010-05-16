/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "eann/ga.h"

#include <algorithm>
#include <limits>

#include <boost/foreach.hpp>

namespace HiveMind {

inline float randFloat()
{
  return rand() / (RAND_MAX + 1.0);
}

inline float randWeight()
{
  return randFloat() - randFloat();
}

inline int randInt(int x, int y)
{
  return rand() % (y - x + 1) + x;
}

Genome::Genome()
  : m_fitness(0)
{
}

Genome::Genome(std::vector<float> weights, float fitness)
  : m_weights(weights),
    m_fitness(fitness)
{
}

void Genome::setRandomWeights(int count)
{
  for (int i = 0; i < count; i++) {
    m_weights.push_back(randWeight());
  }
}

GeneticAlgorithm::GeneticAlgorithm(int populationSize, int numWeights, float mutationRate, float crossRate)
  : m_populationSize(populationSize),
    m_numWeights(numWeights),
    m_mutationRate(mutationRate),
    m_crossoverRate(crossRate),
    m_totalFitness(0),
    m_bestFitness(0),
    m_averageFitness(0),
    m_worstFitness(std::numeric_limits<float>::infinity()),
    m_generation(0)
{
  // Initialize a population of random weights
  for (int i = 0; i < m_populationSize; i++) {
    m_population.push_back(Genome());
    m_population[i].setRandomWeights(numWeights);
  }
} 

void GeneticAlgorithm::evolve()
{
  // Sort population by fitness (for elitism)
  std::sort(m_population.begin(), m_population.end());
  
  // Update best/worst/avg statistics
  calculateStatistics();
  
  // Create new population
  std::vector<Genome> newp;
  selectNBest(4, 1, newp);
  
  while (newp.size() < m_populationSize) {
    // Select parents
    Genome mum = selectByRoulette();
    Genome dad = selectByRoulette();
    
    // Create offspring via crossover
    std::vector<float> child1, child2;
    crossover(mum.getWeights(), dad.getWeights(), child1, child2);
    
    // Inflict mutations on poor kids
    mutate(child1);
    mutate(child2);
    
    // Now insert new genomes into the population
    newp.push_back(Genome(child1, 0));
    newp.push_back(Genome(child2, 0));
  }
  
  m_population = newp;
}

void GeneticAlgorithm::crossover(const std::vector<float> &mum, const std::vector<float> &dad,
                                 std::vector<float> &child1, std::vector<float> &child2)
{
  // Return parents as children when we are below crossover rate or parents are the same
  if (randFloat() > m_crossoverRate || mum == dad) {
    child1 = mum;
    child2 = dad;
    return;
  }
  
  // Determine a crossover point
  int cp = randInt(0, m_numWeights - 1);
  
  // Create the offspring
  for (int i = 0; i  < cp; i++) {
    child1.push_back(mum[i]);
    child2.push_back(dad[i]);
  }
  
  for (int i = cp; i < mum.size(); i++) {
    child1.push_back(dad[i]);
    child1.push_back(mum[i]);
  }
}

void GeneticAlgorithm::mutate(std::vector<float> &weights)
{
  BOOST_FOREACH(float &weight, weights) {
    if (randFloat() < m_mutationRate) {
      weight += randWeight() * 0.3;
    }
  }
}

Genome GeneticAlgorithm::selectByRoulette()
{
  Genome genome;
  float slice = (float) (randFloat() * m_totalFitness);
  float fitness = 0;
  
  BOOST_FOREACH(Genome &g, m_population) {
    fitness += g.getFitness();
    if (fitness >= slice) {
      genome = g;
      break;
    }
  }
  
  return genome;
}

void GeneticAlgorithm::selectNBest(int n, const int copies, Population &population)
{
  while (n--) {
    for (int i = 0; i < copies; i++) {
      population.push_back(m_population[(m_populationSize - 1) - n]);
    }
  }
}

void GeneticAlgorithm::calculateStatistics()
{
  m_totalFitness = 0;
  m_bestFitness = 0;
  m_worstFitness = std::numeric_limits<float>::infinity();
  m_averageFitness = 0;
  
  // Iterate thorugh the population and update best/worst fitness
  BOOST_FOREACH(Genome &g, m_population) {
    if (g.getFitness() > m_bestFitness) {
      m_bestFitness = g.getFitness();
      m_fittestGenome = g;
    }
    
    if (g.getFitness() < m_worstFitness) {
      m_worstFitness = g.getFitness();
    }
    
    m_totalFitness += g.getFitness();
  }
  
  m_averageFitness = m_totalFitness / m_populationSize;
}

}


