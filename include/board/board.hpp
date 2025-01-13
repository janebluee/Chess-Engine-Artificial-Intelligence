#pragma once

#include <array>
#include <string>
#include <cstdint>

class Board {
public:
    static constexpr int WHITE = 0;
    static constexpr int BLACK = 1;
    
    Board();
    
    void setFromFEN(const std::string& fen);
    std::string getFEN() const;
    bool makeMove(uint16_t move);
    void unmakeMove(uint16_t move);
    bool isGameOver() const;
    int getSideToMove() const;
    
    const std::array<uint64_t, 12>& getPieces() const;
    uint64_t getOccupied() const;
    int getCastlingRights() const;
    int getEnPassantSquare() const;
    
private:
    std::array<uint64_t, 12> pieces;
    uint64_t occupied;
    int sideToMove;
    int castlingRights;
    int enPassantSquare;
    int halfMoveClock;
    int fullMoveNumber;
    
    struct UndoInfo {
        uint16_t move;
        int castlingRights;
        int enPassantSquare;
        int halfMoveClock;
        uint64_t hash;
    };
    
    std::vector<UndoInfo> history;
    
    void clearBoard();
    void updateOccupied();
    bool isAttacked(int square, int side) const;
    void placePiece(int piece, int square);
    void removePiece(int piece, int square);
};
