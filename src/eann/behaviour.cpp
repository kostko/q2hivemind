/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "eann/behaviour.h"

namespace HiveMind {

EvolvedBehaviour::EvolvedBehaviour(int numInputs, int numOutputs, int numHiddenLayers, int numNeuronsPerLayer,
                                   int populationSize, float mutationRate, float crossRate)
  : m_neuralNet(numInputs, numOutputs, numHiddenLayers, numNeuronsPerLayer),
    m_ga(populationSize, m_neuralNet.getNumWeights(), mutationRate, crossRate),
    m_currentGenome(-1),
    m_frozen(false)
{
}

void EvolvedBehaviour::evaluate()
{
  // When evolution is frozen we are stuck with the fittest genome
  if (m_frozen) {
    m_neuralNet.setWeights(m_ga.getFittestGenome().getWeights());    
    return;
  }
  
  // Set current genome fitness accoording to behaviour fitness function
  if (m_currentGenome > -1)
    m_ga.getPopulation()[m_currentGenome].setFitness(fitness());
  
  // Switch to the next genome
  m_currentGenome++;
  if (m_currentGenome >= m_ga.getPopulation().size()) {
    // No further genomes to test, since fitness has been evaluated we
    // evolve the population and start from the beginning
    m_ga.evolve();
    m_currentGenome = 0;
  }
  
  // Replace behaviour ANN weights
  m_neuralNet.setWeights(m_ga.getPopulation()[m_currentGenome].getWeights());
}

std::vector<float> EvolvedBehaviour::behave(const std::vector<float> &inputs)
{
  return m_neuralNet.update(inputs);
}

void EvolvedBehaviour::save(const std::string &filename)
{
  // TODO
}

void EvolvedBehaviour::load(const std::string &filename)
{
  // TODO
}

void EvolvedBehaviour::freeze()
{
  m_frozen = true;
}

void EvolvedBehaviour::unfreeze()
{
  m_frozen = false;
}

}


