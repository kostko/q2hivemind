/*
 * This file is part of HiveMind distributed Quake 2 bot.
 *
 * Copyright (C) 2010 by Jernej Kos <kostko@unimatrix-one.org>
 * Copyright (C) 2010 by Anze Vavpetic <anze.vavpetic@gmail.com>
 * Copyright (C) 2010 by Grega Kespret <grega.kespret@gmail.com>
 */
#include "eann/ann.h"

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

Neuron::Neuron(int numInputs)
{
  for (int i = 0; i < numInputs + 1; i++) {
    weights.push_back(randWeight());
  }
}

NeuronLayer::NeuronLayer(int numNeurons, int numInputsPerNeuron)
{
  for (int i = 0; i < numNeurons; i++) {
    neurons.push_back(Neuron(numInputsPerNeuron));
  }
}

NeuralNet::NeuralNet(int numInputs, int numOutputs, int numHiddenLayers, int numNeuronsPerLayer)
  : m_numInputs(numInputs),
    m_numOutputs(numOutputs),
    m_numHiddenLayers(numHiddenLayers),
    m_neuronsPerLayer(numNeuronsPerLayer)
{
  // Prepare the initial random neural network
  initialize();
}

void NeuralNet::initialize()
{
  m_layers.clear();
  
  if (m_numHiddenLayers > 0) {
    // Create first hidden layer
    m_layers.push_back(NeuronLayer(m_neuronsPerLayer, m_numInputs));
    
    for (int i = 0; i < m_numHiddenLayers - 1; i++) {
      m_layers.push_back(NeuronLayer(m_neuronsPerLayer, m_neuronsPerLayer));
    }
    
    // Create output layer
    m_layers.push_back(NeuronLayer(m_numOutputs, m_neuronsPerLayer));
  } else {
    // Create output layer
    m_layers.push_back(NeuronLayer(m_numOutputs, m_numInputs));
  }
}

std::vector<float> NeuralNet::getWeights() const
{
  std::vector<float> weights;
  
  BOOST_FOREACH(NeuronLayer layer, m_layers) {
    BOOST_FOREACH(Neuron neuron, layer.neurons) {
      BOOST_FOREACH(float weight, neuron.weights) {
        weights.push_back(weight);
      }
    }
  }
  
  return weights;
}

int NeuralNet::getNumWeights() const
{
  int weights;
  
  BOOST_FOREACH(NeuronLayer layer, m_layers) {
    BOOST_FOREACH(Neuron neuron, layer.neurons) {
      BOOST_FOREACH(float weight, neuron.weights) {
        weights++;
      }
    }
  }
  
  return weights;
}

void NeuralNet::setWeights(const std::vector<float> &weights)
{
  int w = 0;
  
  BOOST_FOREACH(NeuronLayer &layer, m_layers) {
    BOOST_FOREACH(Neuron &neuron, layer.neurons) {
      BOOST_FOREACH(float &weight, neuron.weights) {
        weight = weights[w++];
      }
    }
  }
}

std::vector<float> NeuralNet::update(std::vector<float> inputs)
{
  std::vector<float> outputs;
  int w = 0;
  
  // Sanity check for number of inputs
  if (inputs.size() != m_numInputs) {
    return outputs;
  }
  
  for (int i = 0; i < m_numHiddenLayers + 1; i++) {
    // Transfer outputs of previous layer to next layer
    if (i > 0) {
      inputs = outputs;
    }
    
    outputs.clear();
    BOOST_FOREACH(Neuron &neuron, m_layers[i].neurons) {
      float in = 0;
      int numInputs = neuron.weights.size();
      int w = 0;
      // Compute the weighted sum
      for (int j = 0; j < numInputs - 1; j++) {
        in += neuron.weights[j] * inputs[w++];
      }
      
      // Add bias
      in += neuron.weights[numInputs - 1] * -1;
      
      // Insert outputs
      outputs.push_back(sigmoid(in, 1));
    }
  }
  
  return outputs;
}

}


