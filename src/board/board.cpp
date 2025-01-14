#include "../../include/board/board.hpp"
#include <sstream>
#include <cctype>
#include <bitset>

namespace {
    constexpr uint64_t KING_CASTLE_MASK_WHITE = 0x60ULL;
    constexpr uint64_t QUEEN_CASTLE_MASK_WHITE = 0xEULL;
    constexpr uint64_t KING_CASTLE_MASK_BLACK = 0x6000000000000000ULL;
    constexpr uint64_t QUEEN_CASTLE_MASK_BLACK = 0xE00000000000000ULL;
    constexpr uint64_t RANK_1 = 0xFFULL;
    constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;
    constexpr uint64_t FILE_A = 0x0101010101010101ULL;
    constexpr uint64_t FILE_H = 0x8080808080808080ULL;

    inline bool isSquareAttacked(const std::array<uint64_t, 12>& pieces, int square, int attackingSide, uint64_t occupied) {
        uint64_t pawnMask = 1ULL << square;
        if (attackingSide == Board::WHITE) {
            pawnMask = ((pawnMask & ~FILE_A) >> 9) | ((pawnMask & ~FILE_H) >> 7);
            if (pawnMask & pieces[Board::BLACK * 6 + Board::PAWN]) return true;
        } else {
            pawnMask = ((pawnMask & ~FILE_A) << 7) | ((pawnMask & ~FILE_H) << 9);
            if (pawnMask & pieces[Board::WHITE * 6 + Board::PAWN]) return true;
        }

        uint64_t knightMask = 1ULL << square;
        uint64_t knightAttacks = ((knightMask & ~FILE_A & ~FILE_B) >> 10) |
                                ((knightMask & ~FILE_A) >> 17) |
                                ((knightMask & ~FILE_H) >> 15) |
                                ((knightMask & ~FILE_G & ~FILE_H) >> 6) |
                                ((knightMask & ~FILE_A & ~FILE_B) << 6) |
                                ((knightMask & ~FILE_A) << 15) |
                                ((knightMask & ~FILE_H) << 17) |
                                ((knightMask & ~FILE_G & ~FILE_H) << 10);
        if (knightAttacks & pieces[attackingSide * 6 + Board::KNIGHT]) return true;

        uint64_t bishopAttacks = 0;
        int sq = square;
        for (int r = sq + 9; r < 64 && r % 8 > sq % 8; r += 9) {
            bishopAttacks |= 1ULL << r;
            if (occupied & (1ULL << r)) break;
        }
        for (int r = sq + 7; r < 64 && r % 8 < sq % 8; r += 7) {
            bishopAttacks |= 1ULL << r;
            if (occupied & (1ULL << r)) break;
        }
        for (int r = sq - 7; r >= 0 && r % 8 > sq % 8; r -= 7) {
            bishopAttacks |= 1ULL << r;
            if (occupied & (1ULL << r)) break;
        }
        for (int r = sq - 9; r >= 0 && r % 8 < sq % 8; r -= 9) {
            bishopAttacks |= 1ULL << r;
            if (occupied & (1ULL << r)) break;
        }
        if (bishopAttacks & (pieces[attackingSide * 6 + Board::BISHOP] | pieces[attackingSide * 6 + Board::QUEEN])) return true;

        uint64_t rookAttacks = 0;
        for (int r = sq + 8; r < 64; r += 8) {
            rookAttacks |= 1ULL << r;
            if (occupied & (1ULL << r)) break;
        }
        for (int r = sq - 8; r >= 0; r -= 8) {
            rookAttacks |= 1ULL << r;
            if (occupied & (1ULL << r)) break;
        }
        for (int r = sq + 1; r % 8 != 0; r++) {
            rookAttacks |= 1ULL << r;
            if (occupied & (1ULL << r)) break;
        }
        for (int r = sq - 1; r % 8 != 7 && r >= 0; r--) {
            rookAttacks |= 1ULL << r;
            if (occupied & (1ULL << r)) break;
        }
        if (rookAttacks & (pieces[attackingSide * 6 + Board::ROOK] | pieces[attackingSide * 6 + Board::QUEEN])) return true;

        uint64_t kingMask = 1ULL << square;
        uint64_t kingAttacks = ((kingMask & ~FILE_A) >> 1) |
                              ((kingMask & ~FILE_H) << 1) |
                              (kingMask << 8) |
                              (kingMask >> 8) |
                              ((kingMask & ~FILE_A) >> 9) |
                              ((kingMask & ~FILE_H) >> 7) |
                              ((kingMask & ~FILE_A) << 7) |
                              ((kingMask & ~FILE_H) << 9);
        if (kingAttacks & pieces[attackingSide * 6 + Board::KING]) return true;

        return false;
    }
}

Board::Board() {
    clearBoard();
}

void Board::setFromFEN(const std::string& fen) {
    clearBoard();

    std::istringstream iss(fen);
    std::string board, side, castling, enPassant, halfMove, fullMove;
    iss >> board >> side >> castling >> enPassant >> halfMove >> fullMove;

    int rank = 7;
    int file = 0;

    for (char c : board) {
        if (c == '/') {
            rank--;
            file = 0;
        } else if (std::isdigit(c)) {
            file += static_cast<int>(c - '0');
        } else {
            int piece = getPieceFromChar(c);
            int square = rank * 8 + file;
            placePiece(piece, square);
            file++;
        }
    }

    sideToMove = (side == "b") ? BLACK : WHITE;

    castlingRights = 0;
    for (char c : castling) {
        switch (c) {
            case 'K': castlingRights |= 1; break;
            case 'Q': castlingRights |= 2; break;
            case 'k': castlingRights |= 4; break;
            case 'q': castlingRights |= 8; break;
        }
    }

    enPassantSquare = (enPassant == "-") ? -1 : 
        (enPassant[0] - 'a') + ((enPassant[1] - '1') * 8);

    halfMoveClock = std::stoi(halfMove);
    fullMoveNumber = std::stoi(fullMove);

    updateOccupied();
}

std::string Board::getFEN() const {
    std::string fen;

    for (int rank = 7; rank >= 0; rank--) {
        int emptyCount = 0;

        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            bool found = false;

            for (int piece = 0; piece < 12; piece++) {
                if (pieces[piece] & (1ULL << square)) {
                    if (emptyCount > 0) {
                        fen += std::to_string(emptyCount);
                        emptyCount = 0;
                    }
                    fen += getCharFromPiece(piece);
                    found = true;
                    break;
                }
            }

            if (!found) {
                emptyCount++;
            }
        }

        if (emptyCount > 0) {
            fen += std::to_string(emptyCount);
        }

        if (rank > 0) {
            fen += '/';
        }
    }

    fen += ' ';
    fen += (sideToMove == BLACK) ? 'b' : 'w';
    fen += ' ';

    std::string castling;
    if (castlingRights & 1) castling += 'K';
    if (castlingRights & 2) castling += 'Q';
    if (castlingRights & 4) castling += 'k';
    if (castlingRights & 8) castling += 'q';
    fen += castling.empty() ? "-" : castling;

    fen += ' ';
    if (enPassantSquare == -1) {
        fen += '-';
    } else {
        fen += static_cast<char>('a' + (enPassantSquare % 8));
        fen += static_cast<char>('1' + (enPassantSquare / 8));
    }

    fen += ' ';
    fen += std::to_string(halfMoveClock);
    fen += ' ';
    fen += std::to_string(fullMoveNumber);

    return fen;
}

void Board::clearBoard() {
    for (auto& bb : pieces) {
        bb = 0;
    }
    occupied = 0;
    sideToMove = WHITE;
    castlingRights = 0;
    enPassantSquare = -1;
    halfMoveClock = 0;
    fullMoveNumber = 1;
    history.clear();
}

void Board::updateOccupied() {
    occupied = 0;
    for (const auto& bb : pieces) {
        occupied |= bb;
    }
}

void Board::placePiece(int piece, int square) {
    pieces[piece] |= (1ULL << square);
    occupied |= (1ULL << square);
}

void Board::removePiece(int piece, int square) {
    pieces[piece] &= ~(1ULL << square);
    occupied &= ~(1ULL << square);
}

int Board::getPieceFromChar(char c) {
    int piece = -1;
    bool isBlack = std::islower(static_cast<unsigned char>(c));
    unsigned char p = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c)));

    switch (p) {
        case 'p': piece = PAWN; break;
        case 'n': piece = KNIGHT; break;
        case 'b': piece = BISHOP; break;
        case 'r': piece = ROOK; break;
        case 'q': piece = QUEEN; break;
        case 'k': piece = KING; break;
    }

    return piece + (isBlack ? 6 : 0);
}

char Board::getCharFromPiece(int piece) {
    static const char pieces[] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k'};
    return pieces[piece];
}

bool Board::makeMove(uint16_t move) {
    const int from = move & 0x3F;
    const int to = (move >> 6) & 0x3F;
    const int promotion = (move >> 12) & 0x7;

    UndoInfo undo{move, castlingRights, enPassantSquare, 0};

    int movingPiece = -1;
    for (int p = sideToMove * 6; p < (sideToMove + 1) * 6; ++p) {
        if (pieces[p] & (1ULL << from)) {
            movingPiece = p;
            break;
        }
    }
    if (movingPiece == -1) return false;

    for (int p = (!sideToMove) * 6; p < (!sideToMove + 1) * 6; ++p) {
        if (pieces[p] & (1ULL << to)) {
            removePiece(p, to);
            break;
        }
    }

    removePiece(movingPiece, from);
    if (promotion) {
        placePiece(sideToMove * 6 + promotion, to);
    } else {
        placePiece(movingPiece, to);
    }

    if (movingPiece % 6 == KING) {
        if (from == (sideToMove ? 60 : 4)) {
            if (to == (sideToMove ? 62 : 6)) {
                removePiece(sideToMove * 6 + ROOK, sideToMove ? 63 : 7);
                placePiece(sideToMove * 6 + ROOK, sideToMove ? 61 : 5);
            } else if (to == (sideToMove ? 58 : 2)) {
                removePiece(sideToMove * 6 + ROOK, sideToMove ? 56 : 0);
                placePiece(sideToMove * 6 + ROOK, sideToMove ? 59 : 3);
            }
        }
        castlingRights &= ~(3 << (sideToMove * 2));
    }

    if (from == 0 || to == 0) castlingRights &= ~2;
    if (from == 7 || to == 7) castlingRights &= ~1;
    if (from == 56 || to == 56) castlingRights &= ~8;
    if (from == 63 || to == 63) castlingRights &= ~4;

    if (movingPiece % 6 == PAWN) {
        if (abs(to - from) == 16) {
            enPassantSquare = (from + to) / 2;
        } else if (to == undo.enPassantSquare) {
            removePiece((!sideToMove) * 6 + PAWN, to + (sideToMove ? -8 : 8));
        }
    } else {
        enPassantSquare = -1;
    }

    if (sideToMove == BLACK) {
        ++fullMoveNumber;
    }

    if (movingPiece % 6 == PAWN || (occupied & (1ULL << to))) {
        halfMoveClock = 0;
    } else {
        ++halfMoveClock;
    }

    history.push_back(undo);
    sideToMove = !sideToMove;

    int kingSquare = 0;
    uint64_t kingBB = pieces[(!sideToMove) * 6 + KING];
    while (kingBB) {
        kingSquare = __builtin_ctzll(kingBB);
        break;
    }

    if (isSquareAttacked(pieces, kingSquare, sideToMove, occupied)) {
        unmakeMove(move);
        return false;
    }

    return true;
}

void Board::unmakeMove(uint16_t move) {
    if (history.empty()) return;

    sideToMove = !sideToMove;

    const int from = move & 0x3F;
    const int to = (move >> 6) & 0x3F;
    const int promotion = (move >> 12) & 0x7;

    int movingPiece = -1;
    if (promotion) {
        movingPiece = sideToMove * 6 + PAWN;
    } else {
        for (int p = sideToMove * 6; p < (sideToMove + 1) * 6; ++p) {
            if (pieces[p] & (1ULL << to)) {
                movingPiece = p;
                break;
            }
        }
    }

    removePiece(promotion ? sideToMove * 6 + promotion : movingPiece, to);
    placePiece(movingPiece, from);

    auto& undo = history.back();

    if (movingPiece % 6 == KING) {
        if (from == (sideToMove ? 60 : 4)) {
            if (to == (sideToMove ? 62 : 6)) {
                removePiece(sideToMove * 6 + ROOK, sideToMove ? 61 : 5);
                placePiece(sideToMove * 6 + ROOK, sideToMove ? 63 : 7);
            } else if (to == (sideToMove ? 58 : 2)) {
                removePiece(sideToMove * 6 + ROOK, sideToMove ? 59 : 3);
                placePiece(sideToMove * 6 + ROOK, sideToMove ? 56 : 0);
            }
        }
    }

    if (movingPiece % 6 == PAWN && to == undo.enPassantSquare) {
        placePiece((!sideToMove) * 6 + PAWN, to + (sideToMove ? -8 : 8));
    }

    castlingRights = undo.castlingRights;
    enPassantSquare = undo.enPassantSquare;

    if (sideToMove == BLACK) {
        --fullMoveNumber;
    }

    history.pop_back();
}

bool Board::isInCheck() const {
    int kingSquare = 0;
    uint64_t kingBB = pieces[sideToMove * 6 + KING];
    while (kingBB) {
        kingSquare = __builtin_ctzll(kingBB);
        break;
    }

    return isSquareAttacked(pieces, kingSquare, !sideToMove, occupied);
}

int Board::getSideToMove() const {
    return sideToMove;
}

uint64_t Board::getOccupied() const {
    return occupied;
}

int Board::getEnPassantSquare() const {
    return enPassantSquare;
}

int Board::getPieceAt(int square) const {
    uint64_t mask = 1ULL << square;
    for (int i = 0; i < 12; ++i) {
        if (pieces[i] & mask) {
            return i;
        }
    }
    return -1;
}

std::vector<uint16_t> Board::generateLegalMoves() const {
    MoveGenerator moveGen;
    auto moves = moveGen.generateLegalMoves(pieces, occupied, sideToMove, castlingRights, enPassantSquare);
    
    std::vector<uint16_t> result;
    result.reserve(moves.size());
    for (const auto& move : moves) {
        uint16_t encoded = (move.from & 0x3F) | ((move.to & 0x3F) << 6) | ((move.promotion & 0xF) << 12);
        result.push_back(encoded);
    }
    return result;
}
