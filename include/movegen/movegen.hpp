#pragma once
#include "../board/board.hpp"
#include <vector>
#include <cstdint>

class MoveGenerator {
public:
    static constexpr uint64_t RANK_1 = 0xFFULL;
    static constexpr uint64_t RANK_2 = RANK_1 << 8;
    static constexpr uint64_t RANK_7 = RANK_1 << 48;
    static constexpr uint64_t RANK_8 = RANK_1 << 56;
    
    static constexpr uint64_t FILE_A = 0x0101010101010101ULL;
    static constexpr uint64_t FILE_B = FILE_A << 1;
    static constexpr uint64_t FILE_G = FILE_A << 6;
    static constexpr uint64_t FILE_H = FILE_A << 7;
    
    static constexpr uint64_t NOT_FILE_A = ~FILE_A;
    static constexpr uint64_t NOT_FILE_H = ~FILE_H;
    
    static constexpr uint64_t CENTER = 0x0000001818000000ULL;
    static constexpr uint64_t EXTENDED_CENTER = 0x00003C3C3C3C0000ULL;
    
    static std::vector<uint16_t> generateLegalMoves(const Board& board);
    static std::vector<uint16_t> generatePseudoLegalMoves(const Board& board);
    
private:
    static uint64_t getPawnAttacks(int square, int side);
    static uint64_t getKnightAttacks(int square);
    static uint64_t getBishopAttacks(int square, uint64_t occupied);
    static uint64_t getRookAttacks(int square, uint64_t occupied);
    static uint64_t getQueenAttacks(int square, uint64_t occupied);
    static uint64_t getKingAttacks(int square);
    
    static uint64_t getPawnPushes(int square, int side, uint64_t occupied);
    static uint64_t getPawnDoublePushes(int square, int side, uint64_t occupied);
    static uint64_t getPawnCaptures(int square, int side, uint64_t enemies);
    static uint64_t getPawnEnPassant(int square, int side, int epSquare);
    
    static uint64_t getSliderAttacks(int square, uint64_t occupied, const std::array<uint64_t, 64>& rays);
    static uint64_t getBishopRays(int square);
    static uint64_t getRookRays(int square);
    
    static void addMoves(std::vector<uint16_t>& moves, int from, uint64_t targets);
    static void addPromotions(std::vector<uint16_t>& moves, int from, int to);
    
    static bool isSquareAttacked(const Board& board, int square, int side);
    static bool isKingInCheck(const Board& board);
    static bool isMoveIntoCheck(const Board& board, uint16_t move);
};
