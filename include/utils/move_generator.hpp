#pragma once

#include <cstdint>
#include <array>
#include <vector>

class MoveGenerator {
public:
    struct Move {
        int from;
        int to;
        int promotion;
        int flags;
        
        Move() : from(0), to(0), promotion(0), flags(0) {}
        Move(int f, int t, int p, int fl) : from(f), to(t), promotion(p), flags(fl) {}
        bool operator==(const Move& other) const {
            return from == other.from && to == other.to && promotion == other.promotion && flags == other.flags;
        }
    };

    MoveGenerator() = default;
    ~MoveGenerator() = default;
    
    std::vector<Move> generateLegalMoves(const std::array<uint64_t, 12>& pieces, 
                                       uint64_t occupied,
                                       int sideToMove,
                                       int castlingRights,
                                       int enPassantSquare);
                                       
    std::vector<Move> generateCaptures(const std::array<uint64_t, 12>& pieces, 
                                       uint64_t occupied,
                                       int sideToMove,
                                       int enPassantSquare);
                                       
    bool isAttacked(uint64_t square, int side, uint64_t occupied) const;
    
private:
    static constexpr uint64_t RANK_1 = 0xFF;
    static constexpr uint64_t RANK_8 = 0xFF00000000000000;
    static constexpr uint64_t FILE_A = 0x0101010101010101;
    static constexpr uint64_t FILE_H = 0x8080808080808080;
    
    static constexpr std::array<int, 8> bishopDirections = {-9, -7, 7, 9};
    static constexpr std::array<int, 8> rookDirections = {-8, -1, 1, 8};
    
    std::array<uint64_t, 64> pawnAttacks{};
    std::array<uint64_t, 64> knightMoves{};
    std::array<uint64_t, 64> kingMoves{};
    
    void initializeLookupTables();
    uint64_t generatePawnMoves(int square, int side) const;
    uint64_t generateKnightMoves(int square) const;
    uint64_t generateKingMoves(int square) const;
    uint64_t generateSlidingMoves(int square, const std::array<int, 8>& directions) const;
};
