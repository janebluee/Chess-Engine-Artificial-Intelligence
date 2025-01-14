#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <string>

class NeuralNetwork {
public:
    static constexpr int INPUT_SIZE = 768;
    static constexpr int HIDDEN_SIZE = 512;
    static constexpr int OUTPUT_SIZE = 1;
    
    NeuralNetwork();
    ~NeuralNetwork() = default;
    
    void loadWeights(const std::string& path);
    float forward(const std::array<float, INPUT_SIZE>& input) const;
    void updateAccumulator(int piece, int square, bool add);
    void resetAccumulator();
    
private:
    struct Layer {
        std::vector<float> weights;
        std::vector<float> biases;
        std::vector<float> output;
        
        Layer() = default;
    };
    
    Layer inputLayer;
    Layer hiddenLayer;
    Layer outputLayer;
    
    std::vector<float> accumulator;
    bool accumulatorValid{false};
    
    void initializeWeights();
    static float activateReLU(float x) {
        return x > 0.0f ? x : 0.0f;
    }
    static float activateTanh(float x) {
        return std::tanh(x);
    }
    void computeHiddenLayer(const std::array<float, INPUT_SIZE>& input) const;
    void computeOutputLayer() const;
};
