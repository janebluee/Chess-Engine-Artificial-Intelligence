#include "../../include/eval/evaluator.hpp"
#include <bitset>

int Evaluator::evaluate(const Board& board) {
    int score = 0;
    bool endgame = isEndgame(board);
    
    score += getMaterialCount(board);
    
    for (int piece = 0; piece < 12; ++piece) {
        uint64_t bb = board.pieces[piece];
        while (bb) {
            int square = __builtin_ctzll(bb);
            score += getPieceSquareValue(piece, square, endgame) * (piece < 6 ? 1 : -1);
            bb &= bb - 1;
        }
    }
    
    score += getMobilityScore(board);
    score += getPawnStructureScore(board);
    score += getKingSafetyScore(board);
    
    return board.getSideToMove() == Board::WHITE ? score : -score;
}

int Evaluator::getMaterialCount(const Board& board) {
    int score = 0;
    
    for (int piece = Board::PAWN; piece <= Board::KING; ++piece) {
        int whitePieces = std::bitset<64>(board.pieces[piece]).count();
        int blackPieces = std::bitset<64>(board.pieces[piece + 6]).count();
        
        int value;
        switch (piece) {
            case Board::PAWN: value = PAWN_VALUE; break;
            case Board::KNIGHT: value = KNIGHT_VALUE; break;
            case Board::BISHOP: value = BISHOP_VALUE; break;
            case Board::ROOK: value = ROOK_VALUE; break;
            case Board::QUEEN: value = QUEEN_VALUE; break;
            case Board::KING: value = KING_VALUE; break;
            default: value = 0;
        }
        
        score += (whitePieces - blackPieces) * value;
    }
    
    return score;
}

bool Evaluator::isEndgame(const Board& board) {
    int materialCount = 0;
    
    for (int piece = Board::PAWN; piece <= Board::QUEEN; ++piece) {
        materialCount += std::bitset<64>(board.pieces[piece]).count() * 
            (piece == Board::PAWN ? PAWN_VALUE : 
             piece == Board::KNIGHT ? KNIGHT_VALUE :
             piece == Board::BISHOP ? BISHOP_VALUE :
             piece == Board::ROOK ? ROOK_VALUE : QUEEN_VALUE);
        
        materialCount += std::bitset<64>(board.pieces[piece + 6]).count() * 
            (piece == Board::PAWN ? PAWN_VALUE : 
             piece == Board::KNIGHT ? KNIGHT_VALUE :
             piece == Board::BISHOP ? BISHOP_VALUE :
             piece == Board::ROOK ? ROOK_VALUE : QUEEN_VALUE);
    }
    
    return materialCount <= 2 * ROOK_VALUE + BISHOP_VALUE;
}

int Evaluator::getPieceSquareValue(int piece, int square, bool isEndgame) {
    if (piece >= 6) {
        piece -= 6;
        square = mirrorSquare(square);
    }
    
    switch (piece) {
        case Board::PAWN: return PAWN_PST[square];
        case Board::KNIGHT: return KNIGHT_PST[square];
        case Board::BISHOP: return BISHOP_PST[square];
        case Board::ROOK: return ROOK_PST[square];
        case Board::QUEEN: return QUEEN_PST[square];
        case Board::KING: return isEndgame ? KING_ENDGAME_PST[square] : KING_PST[square];
        default: return 0;
    }
}

int Evaluator::getMobilityScore(const Board& board) {
    int score = 0;
    uint64_t occupied = board.getOccupied();
    
    for (int piece = Board::KNIGHT; piece <= Board::QUEEN; ++piece) {
        uint64_t whitePieces = board.pieces[piece];
        uint64_t blackPieces = board.pieces[piece + 6];
        
        while (whitePieces) {
            int square = __builtin_ctzll(whitePieces);
            score += 5 * std::bitset<64>(board.pieces[piece]).count();
            whitePieces &= whitePieces - 1;
        }
        
        while (blackPieces) {
            int square = __builtin_ctzll(blackPieces);
            score -= 5 * std::bitset<64>(board.pieces[piece + 6]).count();
            blackPieces &= blackPieces - 1;
        }
    }
    
    return score;
}

int Evaluator::getPawnStructureScore(const Board& board) {
    int score = 0;
    
    uint64_t whitePawns = board.pieces[Board::PAWN];
    uint64_t blackPawns = board.pieces[Board::PAWN + 6];
    
    while (whitePawns) {
        int square = __builtin_ctzll(whitePawns);
        if (isIsolatedPawn(board, square)) score -= 20;
        if (isDoubledPawn(board, square)) score -= 10;
        if (isPassedPawn(board, square)) score += 30;
        whitePawns &= whitePawns - 1;
    }
    
    while (blackPawns) {
        int square = __builtin_ctzll(blackPawns);
        if (isIsolatedPawn(board, square)) score += 20;
        if (isDoubledPawn(board, square)) score += 10;
        if (isPassedPawn(board, square)) score -= 30;
        blackPawns &= blackPawns - 1;
    }
    
    score += (getOpenFileCount(board, Board::WHITE) - getOpenFileCount(board, Board::BLACK)) * 10;
    score += (getSemiOpenFileCount(board, Board::WHITE) - getSemiOpenFileCount(board, Board::BLACK)) * 5;
    
    return score;
}

int Evaluator::getKingSafetyScore(const Board& board) {
    int score = 0;
    
    score += getKingShieldScore(board, Board::WHITE);
    score -= getKingShieldScore(board, Board::BLACK);
    
    return score;
}

int Evaluator::mirrorSquare(int square) {
    return square ^ 56;
}

int Evaluator::getOpenFileCount(const Board& board, int side) {
    int count = 0;
    uint64_t pawns = board.pieces[Board::PAWN] | board.pieces[Board::PAWN + 6];
    
    for (int file = 0; file < 8; ++file) {
        uint64_t fileMask = 0x0101010101010101ULL << file;
        if (!(pawns & fileMask)) {
            if (board.pieces[side * 6 + Board::ROOK] & fileMask) count++;
            if (board.pieces[side * 6 + Board::QUEEN] & fileMask) count++;
        }
    }
    
    return count;
}

int Evaluator::getSemiOpenFileCount(const Board& board, int side) {
    int count = 0;
    uint64_t ownPawns = board.pieces[side * 6 + Board::PAWN];
    uint64_t enemyPawns = board.pieces[(!side) * 6 + Board::PAWN];
    
    for (int file = 0; file < 8; ++file) {
        uint64_t fileMask = 0x0101010101010101ULL << file;
        if (!(ownPawns & fileMask) && (enemyPawns & fileMask)) {
            if (board.pieces[side * 6 + Board::ROOK] & fileMask) count++;
            if (board.pieces[side * 6 + Board::QUEEN] & fileMask) count++;
        }
    }
    
    return count;
}

bool Evaluator::isIsolatedPawn(const Board& board, int square) {
    int file = square % 8;
    uint64_t adjacentFiles = 0;
    
    if (file > 0) adjacentFiles |= 0x0101010101010101ULL << (file - 1);
    if (file < 7) adjacentFiles |= 0x0101010101010101ULL << (file + 1);
    
    uint64_t ownPawns = board.pieces[square < 32 ? Board::PAWN : Board::PAWN + 6];
    return !(ownPawns & adjacentFiles);
}

bool Evaluator::isDoubledPawn(const Board& board, int square) {
    int file = square % 8;
    uint64_t fileMask = 0x0101010101010101ULL << file;
    uint64_t ownPawns = board.pieces[square < 32 ? Board::PAWN : Board::PAWN + 6];
    return std::bitset<64>(ownPawns & fileMask).count() > 1;
}

bool Evaluator::isPassedPawn(const Board& board, int square) {
    int file = square % 8;
    int rank = square / 8;
    uint64_t stopSquares = 0;
    
    if (square < 32) {
        for (int r = rank + 1; r < 8; ++r) {
            if (file > 0) stopSquares |= 1ULL << (r * 8 + file - 1);
            stopSquares |= 1ULL << (r * 8 + file);
            if (file < 7) stopSquares |= 1ULL << (r * 8 + file + 1);
        }
        return !(stopSquares & board.pieces[Board::PAWN + 6]);
    } else {
        for (int r = rank - 1; r >= 0; --r) {
            if (file > 0) stopSquares |= 1ULL << (r * 8 + file - 1);
            stopSquares |= 1ULL << (r * 8 + file);
            if (file < 7) stopSquares |= 1ULL << (r * 8 + file + 1);
        }
        return !(stopSquares & board.pieces[Board::PAWN]);
    }
}

int Evaluator::getKingShieldScore(const Board& board, int side) {
    int score = 0;
    uint64_t kingBB = board.pieces[side * 6 + Board::KING];
    if (!kingBB) return 0;
    
    int kingSquare = __builtin_ctzll(kingBB);
    int kingFile = kingSquare % 8;
    int kingRank = kingSquare / 8;
    
    if ((side == Board::WHITE && kingRank == 0) || (side == Board::BLACK && kingRank == 7)) {
        uint64_t shield = 0;
        
        if (kingFile > 0) shield |= 1ULL << (kingSquare + 7 + (side == Board::WHITE ? 8 : -8));
        shield |= 1ULL << (kingSquare + 8 + (side == Board::WHITE ? 8 : -8));
        if (kingFile < 7) shield |= 1ULL << (kingSquare + 9 + (side == Board::WHITE ? 8 : -8));
        
        uint64_t pawns = board.pieces[side * 6 + Board::PAWN];
        score += 10 * std::bitset<64>(shield & pawns).count();
    }
    
    return score;
}
