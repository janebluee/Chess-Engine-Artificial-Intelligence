#pragma once

#include <array>
#include <vector>
#include <cstdint>

class MoveGenerator {
public:
    struct Move {
        uint16_t from : 6;
        uint16_t to : 6;
        uint16_t promotion : 4;
    };

    MoveGenerator();
    
    std::vector<Move> generateLegalMoves(const std::array<uint64_t, 12>& pieces, 
                                       uint64_t occupied,
                                       int side,
                                       int castling,
                                       int enPassant);
                                       
    bool isAttacked(uint64_t square, int side, uint64_t occupied);
    
private:
    static constexpr uint64_t RANK_1 = 0xFF;
    static constexpr uint64_t RANK_8 = 0xFF00000000000000;
    static constexpr uint64_t FILE_A = 0x0101010101010101;
    static constexpr uint64_t FILE_H = 0x8080808080808080;
    
    std::array<uint64_t, 64> pawnAttacks[2];
    std::array<uint64_t, 64> knightMoves;
    std::array<uint64_t, 64> kingMoves;
    
    void initializeLookupTables();
    uint64_t generatePawnMoves(int square, int side);
    uint64_t generateKnightMoves(int square);
    uint64_t generateKingMoves(int square);
    uint64_t generateSlidingMoves(int square, const std::array<int, 8>& directions);
};
