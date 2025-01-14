#include "../../include/endgame/tablebases.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm>

EndgameTablebases::EndgameTablebases() {
    cache.clear();
}

EndgameTablebases::~EndgameTablebases() {}

bool EndgameTablebases::init(const std::string& path) {
    if (!std::filesystem::exists(path)) return false;
    
    baseDir = path;
    initialized = true;
    
    // Verify essential tablebases
    std::vector<std::string> essential = {
        "KPK", "KRK", "KQK", "KBK", "KNK",
        "KBNK", "KBBK", "KRKP", "KQKP"
    };
    
    for (const auto& pieces : essential) {
        std::string tbFile = baseDir + "/" + pieces + ".tb";
        if (!verifyTablebase(tbFile)) {
            initialized = false;
            return false;
        }
    }
    
    return true;
}

bool EndgameTablebases::probeWDL(const Board& board, int& score) {
    if (!initialized) return false;
    
    // Count pieces
    int pieceCount = std::popcount(board.getOccupied());
    if (pieceCount > maxPieces) return false;
    
    // Try cache first
    uint64_t key = board.getHash();
    size_t cacheIdx = key % Cache::size;
    if (cache.entries[cacheIdx].first == key) {
        const auto& entry = cache.entries[cacheIdx].second;
        score = entry.score;
        return true;
    }
    
    // Probe tablebase file
    std::string tbFile = baseDir + "/" + getPiecesId(board) + ".tb";
    if (!std::filesystem::exists(tbFile)) return false;
    
    TablebaseEntry entry;
    if (!probeTable(tbFile, computeIndex(board), entry)) return false;
    
    // Update cache
    cache.entries[cacheIdx] = {key, entry};
    score = entry.score;
    
    return true;
}

bool EndgameTablebases::probeDTZ(const Board& board, int& dtz) {
    if (!initialized) return false;
    
    int score;
    if (!probeWDL(board, score)) return false;
    
    // Try cache first
    uint64_t key = board.getHash();
    size_t cacheIdx = key % Cache::size;
    if (cache.entries[cacheIdx].first == key) {
        dtz = cache.entries[cacheIdx].second.dtz;
        return true;
    }
    
    // Probe DTZ file
    std::string tbFile = baseDir + "/" + getPiecesId(board) + ".dtz";
    if (!std::filesystem::exists(tbFile)) return false;
    
    TablebaseEntry entry;
    if (!probeTable(tbFile, computeIndex(board), entry)) return false;
    
    // Update cache
    cache.entries[cacheIdx] = {key, entry};
    dtz = entry.dtz;
    
    return true;
}

bool EndgameTablebases::probeRoot(const Board& board, uint16_t& bestMove, int& score) {
    if (!initialized) return false;
    
    int wdlScore;
    if (!probeWDL(board, wdlScore)) return false;
    
    // Generate legal moves
    auto moves = board.generateLegalMoves();
    if (moves.empty()) return false;
    
    // Find move that preserves optimal WDL and has best DTZ
    int bestDtz = 255;
    int bestWdl = wdlScore;
    
    for (uint16_t move : moves) {
        Board copy = board;
        copy.makeMove(move);
        
        int moveWdl, moveDtz;
        if (!probeWDL(copy, moveWdl) || !probeDTZ(copy, moveDtz)) continue;
        
        // Negate scores for opponent's perspective
        moveWdl = -moveWdl;
        
        if (moveWdl > bestWdl || (moveWdl == bestWdl && moveDtz < bestDtz)) {
            bestWdl = moveWdl;
            bestDtz = moveDtz;
            bestMove = move;
        }
    }
    
    score = bestWdl;
    return bestMove != 0;
}

bool EndgameTablebases::isTablebaseWin(const Board& board) {
    int score;
    if (!probeWDL(board, score)) return false;
    return score > 0 && std::abs(score) < MATE_BOUND;
}

bool EndgameTablebases::probeTable(const std::string& tbFile, uint64_t idx, TablebaseEntry& entry) {
    std::ifstream file(tbFile, std::ios::binary);
    if (!file) return false;
    
    file.seekg(idx * sizeof(TablebaseEntry));
    file.read(reinterpret_cast<char*>(&entry), sizeof(TablebaseEntry));
    
    return file.good();
}

uint64_t EndgameTablebases::computeIndex(const Board& board) const {
    // Implement perfect hash function for position
    // This is a simplified version, real implementation would be more complex
    uint64_t index = 0;
    uint64_t occupied = board.getOccupied();
    
    while (occupied) {
        int sq = __builtin_ctzll(occupied);
        int piece = board.getPieceAt(sq);
        index = index * 13 + piece + 1;
        occupied &= occupied - 1;
    }
    
    return index;
}

std::string EndgameTablebases::getPiecesId(const Board& board) const {
    std::string id;
    
    // Add white pieces
    for (int p = Board::KING; p <= Board::PAWN; ++p) {
        uint64_t pieces = board.pieces[Board::WHITE * 6 + p];
        while (pieces) {
            int sq = __builtin_ctzll(pieces);
            id += "KQRBNP"[p];
            pieces &= pieces - 1;
        }
    }
    
    // Add black pieces
    id += "v";  // versus
    for (int p = Board::KING; p <= Board::PAWN; ++p) {
        uint64_t pieces = board.pieces[Board::BLACK * 6 + p];
        while (pieces) {
            int sq = __builtin_ctzll(pieces);
            id += "kqrbnp"[p];
            pieces &= pieces - 1;
        }
    }
    
    return id;
}

bool EndgameTablebases::verifyTablebase(const std::string& tbFile) const {
    std::ifstream file(tbFile, std::ios::binary);
    if (!file) return false;
    
    // Read and verify header (simplified)
    char header[8];
    file.read(header, 8);
    
    return std::string(header, 8) == "CHESS TB";
}
