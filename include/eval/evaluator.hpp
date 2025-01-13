#pragma once

#include <array>
#include <memory>
#include "../neural/network.hpp"

class Evaluator {
public:
    Evaluator(std::shared_ptr<NeuralNetwork> nn);
    
    int evaluate(const std::array<uint64_t, 12>& pieces,
                uint64_t occupied,
                int side,
                int gamePhase);
                
    void updateAccumulator(int piece, int square, bool add);
    void resetAccumulator();
    
private:
    static constexpr int PAWN_VALUE = 100;
    static constexpr int KNIGHT_VALUE = 320;
    static constexpr int BISHOP_VALUE = 330;
    static constexpr int ROOK_VALUE = 500;
    static constexpr int QUEEN_VALUE = 900;
    
    std::shared_ptr<NeuralNetwork> network;
    
    std::array<float, NeuralNetwork::INPUT_SIZE> extractFeatures(
        const std::array<uint64_t, 12>& pieces,
        uint64_t occupied,
        int side
    );
    
    int calculateMaterialScore(const std::array<uint64_t, 12>& pieces);
    int calculatePositionalScore(float nnOutput, int gamePhase);
};
