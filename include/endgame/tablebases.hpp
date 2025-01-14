#pragma once
#include "../board/board.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

class EndgameTablebases {
public:
    struct TablebaseEntry {
        int16_t score;      // Score in centipawns
        uint8_t bestMove;   // Best move index
        uint8_t dtz;        // Distance to zero (50-move rule)
        uint8_t wdl;        // Win/Draw/Loss status
    };
    
    enum WDLStatus {
        LOSS = 0,
        BLESSED_LOSS = 1,
        DRAW = 2,
        CURSED_WIN = 3,
        WIN = 4
    };

    EndgameTablebases();
    ~EndgameTablebases();

    // Initialize tablebases
    bool init(const std::string& path);
    
    // Probe WDL (Win/Draw/Loss)
    bool probeWDL(const Board& board, int& score);
    
    // Probe DTZ (Distance to Zero)
    bool probeDTZ(const Board& board, int& dtz);
    
    // Probe root (best move)
    bool probeRoot(const Board& board, uint16_t& bestMove, int& score);
    
    // Check if position is tablebase win
    bool isTablebaseWin(const Board& board);
    
    // Get maximum pieces supported
    int getMaxPieces() const { return maxPieces; }
    
    // Check if tablebases are available
    bool available() const { return initialized; }

private:
    static constexpr int maxPieces = 7;  // Support up to 7-piece tablebases
    bool initialized = false;
    std::string baseDir;
    
    // Cache for frequently accessed positions
    struct Cache {
        static constexpr size_t size = 1024 * 1024;  // 1M entries
        std::array<std::pair<uint64_t, TablebaseEntry>, size> entries;
        void clear() { std::fill(entries.begin(), entries.end(), 
                               std::make_pair(0ULL, TablebaseEntry{})); }
    } cache;
    
    // Internal helpers
    bool probeTable(const std::string& tbFile, uint64_t idx, TablebaseEntry& entry);
    uint64_t computeIndex(const Board& board) const;
    std::string getPiecesId(const Board& board) const;
    bool verifyTablebase(const std::string& tbFile) const;
};
