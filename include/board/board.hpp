#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <intrin.h>
#include "../utils/move_generator.hpp"

#ifdef _MSC_VER
inline int __builtin_ctzll(unsigned long long x) {
    unsigned long index;
    _BitScanForward64(&index, x);
    return static_cast<int>(index);
}
#endif

static constexpr uint64_t FILE_A = 0x0101010101010101ULL;
static constexpr uint64_t FILE_B = 0x0202020202020202ULL;
static constexpr uint64_t FILE_C = 0x0404040404040404ULL;
static constexpr uint64_t FILE_D = 0x0808080808080808ULL;
static constexpr uint64_t FILE_E = 0x1010101010101010ULL;
static constexpr uint64_t FILE_F = 0x2020202020202020ULL;
static constexpr uint64_t FILE_G = 0x4040404040404040ULL;
static constexpr uint64_t FILE_H = 0x8080808080808080ULL;

static constexpr int MATE_BOUND = 30000;

class Board {
public:
    static constexpr int WHITE = 0;
    static constexpr int BLACK = 1;
    
    static constexpr int PAWN = 0;
    static constexpr int KNIGHT = 1;
    static constexpr int BISHOP = 2;
    static constexpr int ROOK = 3;
    static constexpr int QUEEN = 4;
    static constexpr int KING = 5;
    
    static constexpr int FILE_A = 0;
    static constexpr int FILE_B = 1;
    static constexpr int FILE_C = 2;
    static constexpr int FILE_D = 3;
    static constexpr int FILE_E = 4;
    static constexpr int FILE_F = 5;
    static constexpr int FILE_G = 6;
    static constexpr int FILE_H = 7;
    
    Board();
    ~Board() = default;
    
    void setFromFEN(const std::string& fen);
    std::string getFEN() const;
    bool makeMove(uint16_t move);
    void unmakeMove(uint16_t move);
    bool isInCheck() const;
    int getSideToMove() const;
    uint64_t getOccupied() const;
    int getEnPassantSquare() const;
    uint64_t getHash() const { return hash; }
    std::vector<uint16_t> generateLegalMoves() const;
    int getPieceAt(int square) const;
    
private:
    std::array<uint64_t, 12> pieces{};
    uint64_t occupied{0};
    uint64_t hash{0};
    int sideToMove{0};
    int castlingRights{0};
    int enPassantSquare{-1};
    int halfMoveClock{0};
    int fullMoveNumber{0};
    
    struct UndoInfo {
        uint16_t move;
        int castlingRights;
        int enPassantSquare;
        uint64_t hash;
    };
    
    std::vector<UndoInfo> history{};
    
    void clearBoard();
    void updateOccupied();
    void placePiece(int piece, int square);
    void removePiece(int piece, int square);
    static int getPieceFromChar(char c);
    static char getCharFromPiece(int piece);
    
    friend class MoveGenerator;
    friend class Evaluator;
    friend class EndgameTablebases;
};
