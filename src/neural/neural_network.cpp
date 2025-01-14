#include "../../include/neural/neural_network.hpp"
#include <fstream>
#include <random>
#include <immintrin.h>
#include <algorithm>

NeuralNetwork::NeuralNetwork() : accumulatorValid(false) {
    inputLayer.weights.resize(INPUT_SIZE * HIDDEN_SIZE);
    inputLayer.biases.resize(HIDDEN_SIZE);
    inputLayer.output.resize(HIDDEN_SIZE);
    
    hiddenLayer.weights.resize(HIDDEN_SIZE * HIDDEN_SIZE);
    hiddenLayer.biases.resize(HIDDEN_SIZE);
    hiddenLayer.output.resize(HIDDEN_SIZE);
    
    outputLayer.weights.resize(HIDDEN_SIZE * OUTPUT_SIZE);
    outputLayer.biases.resize(OUTPUT_SIZE);
    outputLayer.output.resize(OUTPUT_SIZE);
    
    accumulator.resize(HIDDEN_SIZE);
    initializeWeights();
}

void NeuralNetwork::loadWeights(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return;
    
    file.read(reinterpret_cast<char*>(inputLayer.weights.data()), 
              INPUT_SIZE * HIDDEN_SIZE * sizeof(float));
    file.read(reinterpret_cast<char*>(inputLayer.biases.data()),
              HIDDEN_SIZE * sizeof(float));
    file.read(reinterpret_cast<char*>(hiddenLayer.weights.data()),
              HIDDEN_SIZE * HIDDEN_SIZE * sizeof(float));
    file.read(reinterpret_cast<char*>(hiddenLayer.biases.data()),
              HIDDEN_SIZE * sizeof(float));
    file.read(reinterpret_cast<char*>(outputLayer.weights.data()),
              HIDDEN_SIZE * OUTPUT_SIZE * sizeof(float));
    file.read(reinterpret_cast<char*>(outputLayer.biases.data()),
              OUTPUT_SIZE * sizeof(float));
}

float NeuralNetwork::forward(const std::array<float, INPUT_SIZE>& input) {
    computeHiddenLayer(input);
    computeOutputLayer();
    return outputLayer.output[0];
}

void NeuralNetwork::updateAccumulator(int piece, int square, bool add) {
    const float* weights = inputLayer.weights.data() + (piece * 64 + square) * HIDDEN_SIZE;
    float factor = add ? 1.0f : -1.0f;
    
    __m256 factor_vec = _mm256_set1_ps(factor);
    for (int i = 0; i < HIDDEN_SIZE; i += 8) {
        __m256 acc = _mm256_load_ps(&accumulator[i]);
        __m256 w = _mm256_load_ps(&weights[i]);
        acc = _mm256_fmadd_ps(w, factor_vec, acc);
        _mm256_store_ps(&accumulator[i], acc);
    }
    
    accumulatorValid = true;
}

void NeuralNetwork::resetAccumulator() {
    std::fill(accumulator.begin(), accumulator.end(), 0.0f);
    accumulatorValid = false;
}

void NeuralNetwork::initializeWeights() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(0.0f, 1.0f);
    
    auto initLayer = [&](Layer& layer, int fanIn) {
        float scale = std::sqrt(2.0f / fanIn);
        for (auto& w : layer.weights) w = dist(gen) * scale;
        for (auto& b : layer.biases) b = dist(gen) * scale;
    };
    
    initLayer(inputLayer, INPUT_SIZE);
    initLayer(hiddenLayer, HIDDEN_SIZE);
    initLayer(outputLayer, HIDDEN_SIZE);
}

float NeuralNetwork::activateReLU(float x) {
    return std::max(0.0f, x);
}

float NeuralNetwork::activateTanh(float x) {
    return std::tanh(x);
}

void NeuralNetwork::computeHiddenLayer(const std::array<float, INPUT_SIZE>& input) {
    if (!accumulatorValid) {
        std::fill(accumulator.begin(), accumulator.end(), 0.0f);
        
        for (int i = 0; i < INPUT_SIZE; ++i) {
            if (input[i] == 0.0f) continue;
            
            const float* weights = &inputLayer.weights[i * HIDDEN_SIZE];
            __m256 input_vec = _mm256_set1_ps(input[i]);
            
            for (int j = 0; j < HIDDEN_SIZE; j += 8) {
                __m256 acc = _mm256_load_ps(&accumulator[j]);
                __m256 w = _mm256_load_ps(&weights[j]);
                acc = _mm256_fmadd_ps(w, input_vec, acc);
                _mm256_store_ps(&accumulator[j], acc);
            }
        }
    }
    
    for (int i = 0; i < HIDDEN_SIZE; i += 8) {
        __m256 acc = _mm256_load_ps(&accumulator[i]);
        __m256 bias = _mm256_load_ps(&inputLayer.biases[i]);
        __m256 sum = _mm256_add_ps(acc, bias);
        __m256 activated = _mm256_max_ps(_mm256_setzero_ps(), sum);
        _mm256_store_ps(&inputLayer.output[i], activated);
    }
}

void NeuralNetwork::computeOutputLayer() {
    std::fill(outputLayer.output.begin(), outputLayer.output.end(), 0.0f);
    
    for (int i = 0; i < HIDDEN_SIZE; ++i) {
        if (inputLayer.output[i] == 0.0f) continue;
        
        const float* weights = &outputLayer.weights[i * OUTPUT_SIZE];
        float input_val = inputLayer.output[i];
        
        for (int j = 0; j < OUTPUT_SIZE; ++j) {
            outputLayer.output[j] += input_val * weights[j];
        }
    }
    
    for (int i = 0; i < OUTPUT_SIZE; ++i) {
        outputLayer.output[i] = activateTanh(outputLayer.output[i] + outputLayer.biases[i]);
    }
}
