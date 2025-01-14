#include "../../include/movegen/movegen.hpp"
#include <bitset>

namespace {
    constexpr uint64_t KNIGHT_ATTACKS[64] = {
        0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000A1100ULL, 0x0000000000142200ULL,
        0x0000000000284400ULL, 0x0000000000508800ULL, 0x0000000000A01000ULL, 0x0000000000402000ULL,
        0x0000000002040004ULL, 0x0000000005080008ULL, 0x000000000A110011ULL, 0x0000000014220022ULL,
        0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000A0100010ULL, 0x0000000040200020ULL,
        0x0000000204000402ULL, 0x0000000508000805ULL, 0x0000000A1100110AULL, 0x0000001422002214ULL,
        0x0000002844004428ULL, 0x0000005088008850ULL, 0x000000A0100010A0ULL, 0x0000004020002040ULL,
        0x0000020400040200ULL, 0x0000050800080500ULL, 0x00000A1100110A00ULL, 0x0000142200221400ULL,
        0x0000284400442800ULL, 0x0000508800885000ULL, 0x0000A0100010A000ULL, 0x0000402000204000ULL,
        0x0002040004020000ULL, 0x0005080008050000ULL, 0x000A1100110A0000ULL, 0x0014220022140000ULL,
        0x0028440044280000ULL, 0x0050880088500000ULL, 0x00A0100010A00000ULL, 0x0040200020400000ULL,
        0x0204000402000000ULL, 0x0508000805000000ULL, 0x0A1100110A000000ULL, 0x1422002214000000ULL,
        0x2844004428000000ULL, 0x5088008850000000ULL, 0xA0100010A0000000ULL, 0x4020002040000000ULL,
        0x0400040200000000ULL, 0x0800080500000000ULL, 0x1100110A00000000ULL, 0x2200221400000000ULL,
        0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010A000000000ULL, 0x2000204000000000ULL,
        0x0004020000000000ULL, 0x0008050000000000ULL, 0x00110A0000000000ULL, 0x0022140000000000ULL,
        0x0044280000000000ULL, 0x0088500000000000ULL, 0x0010A00000000000ULL, 0x0020400000000000ULL
    };

    constexpr uint64_t KING_ATTACKS[64] = {
        0x0000000000000302ULL, 0x0000000000000705ULL, 0x0000000000000E0AULL, 0x0000000000001C14ULL,
        0x0000000000003828ULL, 0x0000000000007050ULL, 0x000000000000E0A0ULL, 0x000000000000C040ULL,
        0x0000000000030203ULL, 0x0000000000070507ULL, 0x00000000000E0A0EULL, 0x00000000001C141CULL,
        0x0000000000382838ULL, 0x0000000000705070ULL, 0x0000000000E0A0E0ULL, 0x0000000000C040C0ULL,
        0x0000000003020300ULL, 0x0000000007050700ULL, 0x000000000E0A0E00ULL, 0x000000001C141C00ULL,
        0x0000000038283800ULL, 0x0000000070507000ULL, 0x00000000E0A0E000ULL, 0x00000000C040C000ULL,
        0x0000000302030000ULL, 0x0000000705070000ULL, 0x0000000E0A0E0000ULL, 0x0000001C141C0000ULL,
        0x0000003828380000ULL, 0x0000007050700000ULL, 0x000000E0A0E00000ULL, 0x000000C040C00000ULL,
        0x0000030203000000ULL, 0x0000070507000000ULL, 0x00000E0A0E000000ULL, 0x00001C141C000000ULL,
        0x0000382838000000ULL, 0x0000705070000000ULL, 0x0000E0A0E0000000ULL, 0x0000C040C0000000ULL,
        0x0003020300000000ULL, 0x0007050700000000ULL, 0x000E0A0E00000000ULL, 0x001C141C00000000ULL,
        0x0038283800000000ULL, 0x0070507000000000ULL, 0x00E0A0E000000000ULL, 0x00C040C000000000ULL,
        0x0302030000000000ULL, 0x0705070000000000ULL, 0x0E0A0E0000000000ULL, 0x1C141C0000000000ULL,
        0x3828380000000000ULL, 0x7050700000000000ULL, 0xE0A0E00000000000ULL, 0xC040C00000000000ULL,
        0x0203000000000000ULL, 0x0507000000000000ULL, 0x0A0E000000000000ULL, 0x141C000000000000ULL,
        0x2838000000000000ULL, 0x5070000000000000ULL, 0xA0E0000000000000ULL, 0x40C0000000000000ULL
    };
}

std::vector<uint16_t> MoveGenerator::generateLegalMoves(const Board& board) {
    auto moves = generatePseudoLegalMoves(board);
    moves.erase(
        std::remove_if(moves.begin(), moves.end(),
            [&board](uint16_t move) { return isMoveIntoCheck(board, move); }),
        moves.end()
    );
    return moves;
}

std::vector<uint16_t> MoveGenerator::generatePseudoLegalMoves(const Board& board) {
    std::vector<uint16_t> moves;
    moves.reserve(218);
    
    int side = board.getSideToMove();
    uint64_t occupied = board.getOccupied();
    uint64_t enemies = 0;
    uint64_t friends = 0;
    
    for (int p = 0; p < 6; ++p) {
        friends |= board.pieces[side * 6 + p];
        enemies |= board.pieces[(!side) * 6 + p];
    }
    
    for (int piece = side * 6; piece < (side + 1) * 6; ++piece) {
        uint64_t bb = board.pieces[piece];
        while (bb) {
            int square = __builtin_ctzll(bb);
            uint64_t attacks = 0;
            
            switch (piece % 6) {
                case Board::PAWN:
                    attacks = getPawnCaptures(square, side, enemies);
                    if (board.getEnPassantSquare() != -1) {
                        attacks |= getPawnEnPassant(square, side, board.getEnPassantSquare());
                    }
                    addMoves(moves, square, attacks);
                    
                    uint64_t pushes = getPawnPushes(square, side, occupied);
                    if (pushes && ((side == Board::WHITE && (square >> 3) == 1) || 
                                 (side == Board::BLACK && (square >> 3) == 6))) {
                        pushes |= getPawnDoublePushes(square, side, occupied);
                    }
                    
                    for (uint64_t push = pushes; push; push &= push - 1) {
                        int to = __builtin_ctzll(push);
                        if ((side == Board::WHITE && (to >> 3) == 7) ||
                            (side == Board::BLACK && (to >> 3) == 0)) {
                            addPromotions(moves, square, to);
                        } else {
                            moves.push_back(square | (to << 6));
                        }
                    }
                    break;
                    
                case Board::KNIGHT:
                    attacks = KNIGHT_ATTACKS[square] & ~friends;
                    addMoves(moves, square, attacks);
                    break;
                    
                case Board::BISHOP:
                    attacks = getBishopAttacks(square, occupied) & ~friends;
                    addMoves(moves, square, attacks);
                    break;
                    
                case Board::ROOK:
                    attacks = getRookAttacks(square, occupied) & ~friends;
                    addMoves(moves, square, attacks);
                    break;
                    
                case Board::QUEEN:
                    attacks = getQueenAttacks(square, occupied) & ~friends;
                    addMoves(moves, square, attacks);
                    break;
                    
                case Board::KING:
                    attacks = KING_ATTACKS[square] & ~friends;
                    addMoves(moves, square, attacks);
                    
                    if (!isKingInCheck(board)) {
                        if (side == Board::WHITE) {
                            if ((board.castlingRights & 1) && 
                                !(occupied & 0x60ULL) &&
                                !isSquareAttacked(board, 5, Board::BLACK) &&
                                !isSquareAttacked(board, 6, Board::BLACK)) {
                                moves.push_back(4 | (6 << 6));
                            }
                            if ((board.castlingRights & 2) && 
                                !(occupied & 0xEULL) &&
                                !isSquareAttacked(board, 3, Board::BLACK) &&
                                !isSquareAttacked(board, 2, Board::BLACK)) {
                                moves.push_back(4 | (2 << 6));
                            }
                        } else {
                            if ((board.castlingRights & 4) && 
                                !(occupied & 0x6000000000000000ULL) &&
                                !isSquareAttacked(board, 61, Board::WHITE) &&
                                !isSquareAttacked(board, 62, Board::WHITE)) {
                                moves.push_back(60 | (62 << 6));
                            }
                            if ((board.castlingRights & 8) && 
                                !(occupied & 0x0E00000000000000ULL) &&
                                !isSquareAttacked(board, 59, Board::WHITE) &&
                                !isSquareAttacked(board, 58, Board::WHITE)) {
                                moves.push_back(60 | (58 << 6));
                            }
                        }
                    }
                    break;
            }
            bb &= bb - 1;
        }
    }
    
    return moves;
}

uint64_t MoveGenerator::getPawnAttacks(int square, int side) {
    uint64_t bb = 1ULL << square;
    if (side == Board::WHITE) {
        return ((bb & NOT_FILE_A) << 7) | ((bb & NOT_FILE_H) << 9);
    } else {
        return ((bb & NOT_FILE_A) >> 9) | ((bb & NOT_FILE_H) >> 7);
    }
}

uint64_t MoveGenerator::getKnightAttacks(int square) {
    return KNIGHT_ATTACKS[square];
}

uint64_t MoveGenerator::getBishopAttacks(int square, uint64_t occupied) {
    return getSliderAttacks(square, occupied, {getBishopRays(square)});
}

uint64_t MoveGenerator::getRookAttacks(int square, uint64_t occupied) {
    return getSliderAttacks(square, occupied, {getRookRays(square)});
}

uint64_t MoveGenerator::getQueenAttacks(int square, uint64_t occupied) {
    return getBishopAttacks(square, occupied) | getRookAttacks(square, occupied);
}

uint64_t MoveGenerator::getKingAttacks(int square) {
    return KING_ATTACKS[square];
}

uint64_t MoveGenerator::getPawnPushes(int square, int side, uint64_t occupied) {
    uint64_t bb = 1ULL << square;
    uint64_t target = side == Board::WHITE ? bb << 8 : bb >> 8;
    return target & ~occupied;
}

uint64_t MoveGenerator::getPawnDoublePushes(int square, int side, uint64_t occupied) {
    uint64_t singlePush = getPawnPushes(square, side, occupied);
    if (!singlePush) return 0;
    
    int pushSquare = side == Board::WHITE ? square + 8 : square - 8;
    return getPawnPushes(pushSquare, side, occupied);
}

uint64_t MoveGenerator::getPawnCaptures(int square, int side, uint64_t enemies) {
    return getPawnAttacks(square, side) & enemies;
}

uint64_t MoveGenerator::getPawnEnPassant(int square, int side, int epSquare) {
    return getPawnAttacks(square, side) & (1ULL << epSquare);
}

uint64_t MoveGenerator::getSliderAttacks(int square, uint64_t occupied, const std::array<uint64_t, 64>& rays) {
    uint64_t attacks = rays[square];
    uint64_t blockers = occupied & attacks;
    
    if (blockers) {
        int firstBlocker = side == Board::WHITE ? 
            __builtin_ctzll(blockers) : 63 - __builtin_clzll(blockers);
        attacks ^= rays[firstBlocker];
    }
    
    return attacks;
}

uint64_t MoveGenerator::getBishopRays(int square) {
    uint64_t rays = 0;
    int rank = square / 8;
    int file = square % 8;
    
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f) rays |= 1ULL << (r * 8 + f);
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f) rays |= 1ULL << (r * 8 + f);
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f) rays |= 1ULL << (r * 8 + f);
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f) rays |= 1ULL << (r * 8 + f);
    
    return rays;
}

uint64_t MoveGenerator::getRookRays(int square) {
    uint64_t rays = 0;
    int rank = square / 8;
    int file = square % 8;
    
    for (int r = rank + 1; r < 8; ++r) rays |= 1ULL << (r * 8 + file);
    for (int r = rank - 1; r >= 0; --r) rays |= 1ULL << (r * 8 + file);
    for (int f = file + 1; f < 8; ++f) rays |= 1ULL << (rank * 8 + f);
    for (int f = file - 1; f >= 0; --f) rays |= 1ULL << (rank * 8 + f);
    
    return rays;
}

void MoveGenerator::addMoves(std::vector<uint16_t>& moves, int from, uint64_t targets) {
    while (targets) {
        int to = __builtin_ctzll(targets);
        moves.push_back(from | (to << 6));
        targets &= targets - 1;
    }
}

void MoveGenerator::addPromotions(std::vector<uint16_t>& moves, int from, int to) {
    moves.push_back(from | (to << 6) | (Board::KNIGHT << 12));
    moves.push_back(from | (to << 6) | (Board::BISHOP << 12));
    moves.push_back(from | (to << 6) | (Board::ROOK << 12));
    moves.push_back(from | (to << 6) | (Board::QUEEN << 12));
}

bool MoveGenerator::isSquareAttacked(const Board& board, int square, int side) {
    uint64_t occupied = board.getOccupied();
    
    if (getPawnAttacks(square, !side) & board.pieces[side * 6 + Board::PAWN]) return true;
    if (KNIGHT_ATTACKS[square] & board.pieces[side * 6 + Board::KNIGHT]) return true;
    if (getBishopAttacks(square, occupied) & 
        (board.pieces[side * 6 + Board::BISHOP] | board.pieces[side * 6 + Board::QUEEN])) return true;
    if (getRookAttacks(square, occupied) & 
        (board.pieces[side * 6 + Board::ROOK] | board.pieces[side * 6 + Board::QUEEN])) return true;
    if (KING_ATTACKS[square] & board.pieces[side * 6 + Board::KING]) return true;
    
    return false;
}

bool MoveGenerator::isKingInCheck(const Board& board) {
    int side = board.getSideToMove();
    uint64_t kingBB = board.pieces[side * 6 + Board::KING];
    if (!kingBB) return false;
    
    int kingSquare = __builtin_ctzll(kingBB);
    return isSquareAttacked(board, kingSquare, !side);
}

bool MoveGenerator::isMoveIntoCheck(const Board& board, uint16_t move) {
    Board copy = board;
    copy.makeMove(move);
    return isKingInCheck(copy);
}
