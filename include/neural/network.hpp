#pragma once

#include <vector>
#include <array>
#include <memory>
#include <cstdint>

class NeuralNetwork {
public:
    static constexpr int INPUT_SIZE = 768;
    static constexpr int HIDDEN_SIZE = 512;
    static constexpr int OUTPUT_SIZE = 1;

    NeuralNetwork();
    
    float evaluate(const std::array<float, INPUT_SIZE>& features);
    void loadWeights(const std::string& path);
    void updateAccumulator(int piece, int square, bool add);
    
private:
    struct alignas(64) Layer {
        std::vector<float> weights;
        std::vector<float> biases;
        std::vector<float> output;
    };
    
    std::array<float, INPUT_SIZE> accumulator;
    std::vector<Layer> layers;
    
    void forward();
    float clamp(float x);
};
