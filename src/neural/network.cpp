#include "../../include/neural/network.hpp"
#include <immintrin.h>
#include <algorithm>
#include <cmath>

NeuralNetwork::NeuralNetwork() {
    layers.resize(3);
    
    layers[0].weights.resize(INPUT_SIZE * HIDDEN_SIZE);
    layers[0].biases.resize(HIDDEN_SIZE);
    layers[0].output.resize(HIDDEN_SIZE);
    
    layers[1].weights.resize(HIDDEN_SIZE * HIDDEN_SIZE);
    layers[1].biases.resize(HIDDEN_SIZE);
    layers[1].output.resize(HIDDEN_SIZE);
    
    layers[2].weights.resize(HIDDEN_SIZE * OUTPUT_SIZE);
    layers[2].biases.resize(OUTPUT_SIZE);
    layers[2].output.resize(OUTPUT_SIZE);
    
    std::fill(accumulator.begin(), accumulator.end(), 0.0f);
}

float NeuralNetwork::evaluate(const std::array<float, INPUT_SIZE>& features) {
    __m256 acc = _mm256_setzero_ps();
    
    for (int i = 0; i < INPUT_SIZE; i += 8) {
        __m256 feat = _mm256_load_ps(&features[i]);
        __m256 weights = _mm256_load_ps(&layers[0].weights[i]);
        acc = _mm256_fmadd_ps(feat, weights, acc);
    }
    
    float sum = 0.0f;
    float* acc_ptr = reinterpret_cast<float*>(&acc);
    for (int i = 0; i < 8; ++i) {
        sum += acc_ptr[i];
    }
    
    layers[0].output[0] = std::max(0.0f, sum + layers[0].biases[0]);
    
    forward();
    
    return layers[2].output[0];
}

void NeuralNetwork::forward() {
    for (int layer = 1; layer < 3; ++layer) {
        const auto& prev_output = layers[layer-1].output;
        auto& curr_output = layers[layer].output;
        const auto& weights = layers[layer].weights;
        const auto& biases = layers[layer].biases;
        
        #pragma omp parallel for
        for (int neuron = 0; neuron < static_cast<int>(curr_output.size()); ++neuron) {
            __m256 sum = _mm256_setzero_ps();
            
            for (int prev = 0; prev < static_cast<int>(prev_output.size()); prev += 8) {
                __m256 prev_val = _mm256_load_ps(&prev_output[prev]);
                __m256 weight_val = _mm256_load_ps(&weights[neuron * prev_output.size() + prev]);
                sum = _mm256_fmadd_ps(prev_val, weight_val, sum);
            }
            
            float total = 0.0f;
            float* sum_ptr = reinterpret_cast<float*>(&sum);
            for (int i = 0; i < 8; ++i) {
                total += sum_ptr[i];
            }
            
            curr_output[neuron] = layer == 2 ? std::tanh(total + biases[neuron]) 
                                            : std::max(0.0f, total + biases[neuron]);
        }
    }
}

void NeuralNetwork::updateAccumulator(int piece, int square, bool add) {
    const int feature_index = piece * 64 + square;
    const float factor = add ? 1.0f : -1.0f;
    
    #pragma omp simd
    for (int i = 0; i < INPUT_SIZE; ++i) {
        accumulator[i] += layers[0].weights[feature_index * INPUT_SIZE + i] * factor;
    }
}
