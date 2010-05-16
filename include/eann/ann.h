/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#ifndef HM_EANN_ANN_H
#define HM_EANN_ANN_H

#include <vector>
#include <math.h>

namespace HiveMind {

/**
 * A simple neuron for our ANN.
 */
class Neuron {
public:
    /**
     * Class constructor.
     */
    Neuron(int numInputs);
public:
    // Weights for each input
    std::vector<float> weights;
};

/**
 * Represents a neuron layer (hidden or output) for our ANN.
 */
class NeuronLayer {
public:
    /**
     * Class constructor.
     */
    NeuronLayer(int numNeurons, int numInputsPerNeuron);
public:
    // The actual neurons
    std::vector<Neuron> neurons;
};

/**
 * An aritificial neural network class.
 */
class NeuralNet {
public:
    /**
     * Class constructor.
     *
     * @param numInputs Number of input neurons
     * @param numOutputs Number of output neurons
     * @param numHiddenLayers Number of hidden layers
     * @param numNeuronsPerLayer Number of neurons per hidden layer
     */
    NeuralNet(int numInputs, int numOutputs, int numHiddenLayers, int numNeuronsPerLayer);
    
    /**
     * Initializes the neural network. All weights are set to
     * random values.
     */
    void initialize();
    
    /**
     * Returns current weight vector for the whole ANN.
     */
    std::vector<float> getWeights() const;
    
    /**
     * Returns the number of all weights.
     */
    int getNumWeights() const;
    
    /**
     * Sets a new weight vector for the whole ANN.
     *
     * @param weights New weights in proper order
     */
    void setWeights(const std::vector<float> &weights);
    
    /**
     * Calculates output to some specific input vector.
     *
     * @param inputs Input vector
     */
    std::vector<float> update(std::vector<float> inputs);
protected:
    /**
     * Sigmoid function.
     */
    inline float sigmoid(float input, float response) const
    {
      return (1.0 / (1.0 + exp(-input / response)));
    }
private:
    // Neural net parameters
    int m_numInputs;
    int m_numOutputs;
    int m_numHiddenLayers;
    int m_neuronsPerLayer;
    
    // Actual layers
    std::vector<NeuronLayer> m_layers;
};

}

#endif

