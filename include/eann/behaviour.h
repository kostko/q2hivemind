/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_EANN_BEHAVIOUR_H
#define HM_EANN_BEHAVIOUR_H

#include "eann/ann.h"
#include "eann/ga.h"

#include <vector>
#include <string>

namespace HiveMind {

/**
 * An abstract class for evolved behaviours.
 */
class EvolvedBehaviour {
public:
    /**
     * Class constructor.
     *
     * @param numInputs Number of input neurons
     * @param numOutputs Number of output neurons
     * @param numHiddenLayers Number of hidden layers
     * @param numNeuronsPerLayer Number of neurons per hidden layer
     * @param populationSize Population size
     * @param mutationRate Mutation rate
     * @param crossRate Crossover rate
     */
    EvolvedBehaviour(int numInputs, int numOutputs, int numHiddenLayers, int numNeuronsPerLayer,
                     int populationSize, float mutationRate = 0.08, float crossRate = 0.7);
    
    /**
     * Evaluates the fitness of the current genome and switches on to
     * the next one. When all the genomes have been evaluated a new
     * generation is evolved and the process starts again.
     */
    virtual void evaluate();
    
    /**
     * Returns behavioural outputs for the given inputs. Processing is
     * performed on the active genome.
     *
     * @param inputs Inputs to the active ANN
     * @reutrn Outputs from the active ANN
     */
    std::vector<float> behave(const std::vector<float> &inputs);
    
    /**
     * Saves this behaviour into an external file. The whole population
     * is saved.
     */
    void save(const std::string &filename);
    
    /**
     * Loads this behaviour from an external file.
     */
    void load(const std::string &filename);
    
    /**
     * Freeze evolution and only use the fittest genome for processing.
     */
    void freeze();
    
    /**
     * Unfreeze a previously frozen evolution.
     */
    void unfreeze();
    
    /**
     * Returns true if this behaviour's evolution is currently frozen.
     */
    inline bool isFrozen() const { return m_frozen; }
    
    /**
     * Returns the fitness of the currently active genome.
     */
    float getFitness();
protected:
    /**
     * Reports the fitness for the currently active genome.
     *
     * @param value Fitness value to be set
     */
    void reportFitness(float value);
private:
    // ANN and GA
    NeuralNet m_neuralNet;
    GeneticAlgorithm m_ga;
    
    // Current genome index being evaluated
    int m_currentGenome;
    
    // Freeze state
    bool m_frozen;
};

}

#endif

