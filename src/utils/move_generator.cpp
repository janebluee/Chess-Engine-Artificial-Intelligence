#include "../../include/utils/move_generator.hpp"
#include <algorithm>
#include <cassert>

MoveGenerator::MoveGenerator() {
    initializeLookupTables();
}

void MoveGenerator::initializeLookupTables() {
    // Initialize pawn attacks
    for (int square = 0; square < 64; square++) {
        pawnAttacks[0][square] = generatePawnMoves(square, 0); 
        pawnAttacks[1][square] = generatePawnMoves(square, 1); 
    }

    // Initialize knight moves
    for (int square = 0; square < 64; square++) {
        knightMoves[square] = generateKnightMoves(square);
    }

    // Initialize king moves
    for (int square = 0; square < 64; square++) {
        kingMoves[square] = generateKingMoves(square);
    }
}

uint64_t MoveGenerator::generatePawnMoves(int square, int side) {
    uint64_t moves = 0;
    uint64_t bb = 1ULL << square;
    
    if (side == 0) { // White pawns
        moves |= (bb << 8) & ~FILE_H; // Single push
        if (square < 16) {
            moves |= (bb << 16) & ~FILE_H; // Double push from starting position
        }
        moves |= ((bb << 7) & ~FILE_A) | ((bb << 9) & ~FILE_H); // Captures
    } else { // Black pawns
        moves |= (bb >> 8) & ~FILE_A; // Single push
        if (square >= 48) {
            moves |= (bb >> 16) & ~FILE_A; // Double push from starting position
        }
        moves |= ((bb >> 7) & ~FILE_H) | ((bb >> 9) & ~FILE_A); // Captures
    }
    
    return moves;
}

uint64_t MoveGenerator::generateKnightMoves(int square) {
    uint64_t moves = 0;
    uint64_t bb = 1ULL << square;
    
    moves |= (bb << 17) & ~FILE_A;
    moves |= (bb << 15) & ~FILE_H;
    moves |= (bb << 10) & ~(FILE_A | FILE_B);
    moves |= (bb << 6) & ~(FILE_G | FILE_H);
    moves |= (bb >> 6) & ~(FILE_A | FILE_B);
    moves |= (bb >> 10) & ~(FILE_G | FILE_H);
    moves |= (bb >> 15) & ~FILE_A;
    moves |= (bb >> 17) & ~FILE_H;
    
    return moves;
}

uint64_t MoveGenerator::generateKingMoves(int square) {
    uint64_t moves = 0;
    uint64_t bb = 1ULL << square;
    
    moves |= (bb << 8) & ~FILE_H;
    moves |= (bb << 9) & ~FILE_A;
    moves |= (bb << 7) & ~FILE_H;
    moves |= (bb << 1) & ~FILE_A;
    moves |= (bb >> 1) & ~FILE_H;
    moves |= (bb >> 7) & ~FILE_A;
    moves |= (bb >> 8) & ~FILE_A;
    moves |= (bb >> 9) & ~FILE_H;
    
    return moves;
}

uint64_t MoveGenerator::generateSlidingMoves(int square, const std::array<int, 8>& directions) {
    uint64_t moves = 0;
    uint64_t bb = 1ULL << square;
    
    for (int dir : directions) {
        uint64_t ray = 0;
        uint64_t current = bb;
        
        while (current && !(current & ((dir > 0) ? FILE_H : FILE_A))) {
            current = (dir > 0) ? (current << dir) : (current >> -dir);
            ray |= current;
        }
        
        moves |= ray;
    }
    
    return moves;
}

std::vector<MoveGenerator::Move> MoveGenerator::generateLegalMoves(
    const std::array<uint64_t, 12>& pieces, 
    uint64_t occupied,
    int sideToMove,
    int castlingRights,
    int enPassantSquare) {
    
    std::vector<Move> moves;
    
    // Generate pawn moves
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6] & (1ULL << sq)) {
            uint64_t pawnMoves = generatePawnMoves(sq, sideToMove);
            while (pawnMoves) {
                int to = __builtin_ctzll(pawnMoves);
                moves.push_back(Move(sq, to, 0, 0));
                pawnMoves &= pawnMoves - 1;
            }
        }
    }
    
    // Generate knight moves
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 1] & (1ULL << sq)) {
            uint64_t knightMoves = generateKnightMoves(sq);
            while (knightMoves) {
                int to = __builtin_ctzll(knightMoves);
                moves.push_back(Move(sq, to, 0, 0));
                knightMoves &= knightMoves - 1;
            }
        }
    }
    
    // Generate bishop moves
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 2] & (1ULL << sq)) {
            uint64_t bishopMoves = generateSlidingMoves(sq, bishopDirections);
            while (bishopMoves) {
                int to = __builtin_ctzll(bishopMoves);
                moves.push_back(Move(sq, to, 0, 0));
                bishopMoves &= bishopMoves - 1;
            }
        }
    }
    
    // Generate rook moves
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 3] & (1ULL << sq)) {
            uint64_t rookMoves = generateSlidingMoves(sq, rookDirections);
            while (rookMoves) {
                int to = __builtin_ctzll(rookMoves);
                moves.push_back(Move(sq, to, 0, 0));
                rookMoves &= rookMoves - 1;
            }
        }
    }
    
    // Generate queen moves
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 4] & (1ULL << sq)) {
            uint64_t queenMoves = generateSlidingMoves(sq, bishopDirections) | 
                                generateSlidingMoves(sq, rookDirections);
            while (queenMoves) {
                int to = __builtin_ctzll(queenMoves);
                moves.push_back(Move(sq, to, 0, 0));
                queenMoves &= queenMoves - 1;
            }
        }
    }
    
    // Generate king moves
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 5] & (1ULL << sq)) {
            uint64_t kingMoves = generateKingMoves(sq);
            while (kingMoves) {
                int to = __builtin_ctzll(kingMoves);
                moves.push_back(Move(sq, to, 0, 0));
                kingMoves &= kingMoves - 1;
            }
        }
    }
    
    return moves;
}

std::vector<MoveGenerator::Move> MoveGenerator::generateCaptures(
    const std::array<uint64_t, 12>& pieces,
    uint64_t occupied,
    int sideToMove,
    int enPassantSquare) {
    
    std::vector<Move> moves;
    uint64_t enemyPieces = 0;
    for (int i = 0; i < 6; ++i) {
        enemyPieces |= pieces[(!sideToMove) * 6 + i];
    }
    
    // Generate pawn captures
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6] & (1ULL << sq)) {
            uint64_t pawnMoves = generatePawnMoves(sq, sideToMove) & enemyPieces;
            while (pawnMoves) {
                int to = __builtin_ctzll(pawnMoves);
                moves.push_back(Move(sq, to, 0, 0));
                pawnMoves &= pawnMoves - 1;
            }
        }
    }
    
    // Generate knight captures
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 1] & (1ULL << sq)) {
            uint64_t knightMoves = generateKnightMoves(sq) & enemyPieces;
            while (knightMoves) {
                int to = __builtin_ctzll(knightMoves);
                moves.push_back(Move(sq, to, 0, 0));
                knightMoves &= knightMoves - 1;
            }
        }
    }
    
    // Generate bishop captures
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 2] & (1ULL << sq)) {
            uint64_t bishopMoves = generateSlidingMoves(sq, bishopDirections) & enemyPieces;
            while (bishopMoves) {
                int to = __builtin_ctzll(bishopMoves);
                moves.push_back(Move(sq, to, 0, 0));
                bishopMoves &= bishopMoves - 1;
            }
        }
    }
    
    // Generate rook captures
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 3] & (1ULL << sq)) {
            uint64_t rookMoves = generateSlidingMoves(sq, rookDirections) & enemyPieces;
            while (rookMoves) {
                int to = __builtin_ctzll(rookMoves);
                moves.push_back(Move(sq, to, 0, 0));
                rookMoves &= rookMoves - 1;
            }
        }
    }
    
    // Generate queen captures
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 4] & (1ULL << sq)) {
            uint64_t queenMoves = (generateSlidingMoves(sq, bishopDirections) | 
                                 generateSlidingMoves(sq, rookDirections)) & enemyPieces;
            while (queenMoves) {
                int to = __builtin_ctzll(queenMoves);
                moves.push_back(Move(sq, to, 0, 0));
                queenMoves &= queenMoves - 1;
            }
        }
    }
    
    // Generate king captures
    for (int sq = 0; sq < 64; ++sq) {
        if (pieces[sideToMove * 6 + 5] & (1ULL << sq)) {
            uint64_t kingMoves = generateKingMoves(sq) & enemyPieces;
            while (kingMoves) {
                int to = __builtin_ctzll(kingMoves);
                moves.push_back(Move(sq, to, 0, 0));
                kingMoves &= kingMoves - 1;
            }
        }
    }
    
    return moves;
}

bool MoveGenerator::isAttacked(uint64_t square, int side, uint64_t occupied) {
    // Check pawn attacks
    if (pawnAttacks[!side][__builtin_ctzll(square)] & pieces[side * 6]) {
        return true;
    }
    
    // Check knight attacks
    if (knightMoves[__builtin_ctzll(square)] & pieces[side * 6 + 1]) {
        return true;
    }
    
    // Check king attacks
    if (kingMoves[__builtin_ctzll(square)] & pieces[side * 6 + 5]) {
        return true;
    }
    
    // Check sliding piece attacks (bishops, rooks, queens)
    // This would need to be implemented with proper sliding piece attack generation
    
    return false;
}
