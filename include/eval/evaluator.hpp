#pragma once

#include <cstdint>
#include <array>
#include "../board/board.hpp"

class Evaluator {
public:
    Evaluator() = default;
    ~Evaluator() = default;
    
    int evaluate(const std::array<uint64_t, 12>& pieces, uint64_t occupied, int sideToMove);
    
private:
    static constexpr int PAWN_VALUE = 100;
    static constexpr int KNIGHT_VALUE = 320;
    static constexpr int BISHOP_VALUE = 330;
    static constexpr int ROOK_VALUE = 500;
    static constexpr int QUEEN_VALUE = 900;
    
    int evaluateMaterial(const std::array<uint64_t, 12>& pieces);
    int evaluatePosition(const std::array<uint64_t, 12>& pieces);
    int evaluatePawnStructure(const std::array<uint64_t, 12>& pieces);
    int evaluateMobility(const std::array<uint64_t, 12>& pieces, uint64_t occupied);
    int evaluateKingSafety(const std::array<uint64_t, 12>& pieces, uint64_t occupied);
};
