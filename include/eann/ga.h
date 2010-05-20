/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_EANN_GA_H
#define HM_EANN_GA_H

#include <vector>

namespace HiveMind {

/**
 * A simple genome consisting of weights for the neural
 * net.
 */
class Genome {
public:
    /**
     * Class constructor.
     */
    Genome();
    
    /**
     * Class constructor.
     *
     * @param weights ANN weights
     * @param fitness Current fitness
     */
    Genome(std::vector<float> weights, float fitness);
    
    /**
     * Sets up a number of random weights.
     *
     * @param count How many weights to set up
     */
    void setRandomWeights(int count);
    
    /**
     * Returns the ANN weights.
     */
    inline std::vector<float> &getWeights() { return m_weights; }
    
    /**
     * Returns the fitness of this genome.
     */
    inline float getFitness() const { return m_fitness; }
    
    /**
     * Sets the fitness of this genome.
     *
     * @param fitness Fitness
     */
    inline void setFitness(float fitness) { m_fitness = fitness; }
    
    /**
     * Adds to the fitness of this genome.
     *
     * @param fitness Amount to add
     */
    inline void addFitness(float fitness) { m_fitness += fitness; }
    
    /**
     * Comparison operator for sorting genomes by fitness.
     */
    inline bool operator<(const Genome &other) const
    {
      return m_fitness < other.m_fitness;
    }
private:
    // ANN weights
    std::vector<float> m_weights;
    
    // Genome fitness rank
    float m_fitness;
};

// Define a population type
typedef std::vector<Genome> Population;

/**
 * This class encapsulates a GA evolver used to evolve weights
 * for the artificial neural net.
 */
class GeneticAlgorithm {
public:
    /**
     * Class constructor.
     *
     * @param populationSize Population size
     * @param numWeights Length of the chromosome
     * @param mutationRate Mutation rate
     * @param crossRate Crossover rate
     */
    GeneticAlgorithm(int populationSize, int numWeights, float mutationRate = 0.08, float crossRate = 0.7);
    
    /**
     * Returns the active population.
     */
    inline Population &getPopulation() { return m_population; }
    
    /**
     * Returns the fittest genome.
     */
    inline Genome &getFittestGenome() { return m_fittestGenome; } 
    
    /**
     * Evolves the current population.
     */
    void evolve();
protected:
    /**
     * Performs a genetic crossover operator.
     *
     * @param mum Parent weights
     * @param dad Parent weights
     * @param child1 Where to write child weights
     * @param child2 Where to write child weights
     */
    void crossover(const std::vector<float> &mum, const std::vector<float> &dad,
                   std::vector<float> &child1, std::vector<float> &child2);
    
    /**
     * Performs mutations to weights as accoording to mutation
     * rate.
     *
     * @param weights Weights to mutate
     */
    void mutate(std::vector<float> &weights);
    
    /**
     * Returns a genome based on roulette wheel sampling.
     */
    Genome selectByRoulette();
    
    /**
     * Transforms the fitness values since they are negative and thus
     * unsuitable.
     */
    void transform();
    
    /**
     * Selects N best genomes and copies them into the specified
     * population.
     *
     * @param n Number of best genomes to select
     * @param copies Number of copies
     * @param population Population to insert into
     */
    void selectNBest(int n, const int copies, Population &population);
    
    /**
     * Calculates best/worst/avg fitness statistics.
     */
    void calculateStatistics();
private:
    // GA parameters
    int m_populationSize;
    int m_numWeights;
    float m_mutationRate;
    float m_crossoverRate;
    
    // Statistics
    float m_totalFitness;
    float m_bestFitness;
    float m_averageFitness;
    float m_worstFitness;
    Genome m_fittestGenome;
    int m_generation;
    
    // Active population of genomes
    Population m_population;
};

}

#endif

