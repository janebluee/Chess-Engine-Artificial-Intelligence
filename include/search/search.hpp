#pragma once
#include "../board/board.hpp"
#include "../eval/evaluator.hpp"
#include "../movegen/movegen.hpp"
#include <chrono>
#include <vector>

class Search {
public:
    struct SearchInfo {
        int depth;
        int selDepth;
        uint64_t nodes;
        uint64_t tbHits;
        std::chrono::milliseconds time;
        std::vector<uint16_t> pv;
        int score;
    };

    struct SearchLimits {
        int depth = -1;
        int moveTime = -1;
        uint64_t nodes = 0;
        bool infinite = false;
        std::chrono::milliseconds maxTime{0};
    };

    Search(Board& board, Evaluator& evaluator);
    
    uint16_t getBestMove(const SearchLimits& limits);
    void stopSearch();
    
private:
    static constexpr int MAX_PLY = 128;
    static constexpr int MATE_SCORE = 30000;
    static constexpr int MATE_BOUND = 29000;
    
    struct TTEntry {
        uint64_t key;
        uint16_t move;
        int16_t score;
        uint8_t depth;
        uint8_t bound;
        
        static constexpr uint8_t BOUND_NONE = 0;
        static constexpr uint8_t BOUND_UPPER = 1;
        static constexpr uint8_t BOUND_LOWER = 2;
        static constexpr uint8_t BOUND_EXACT = 3;
    };
    
    Board& board;
    Evaluator& evaluator;
    bool stopped;
    SearchInfo info;
    std::vector<uint16_t> killerMoves[MAX_PLY];
    int historyTable[2][64][64];
    std::vector<TTEntry> tt;
    
    int negamax(int alpha, int beta, int depth, int ply);
    int quiescence(int alpha, int beta, int ply);
    bool isRepetition();
    void updateKillers(uint16_t move, int ply);
    void updateHistory(uint16_t move, int depth);
    TTEntry* probeTT(uint64_t key);
    void storeTT(uint64_t key, uint16_t move, int score, int depth, uint8_t bound);
    std::vector<uint16_t> orderMoves(const std::vector<uint16_t>& moves, uint16_t ttMove, int ply);
    int scoreMove(uint16_t move, uint16_t ttMove, int ply);
    bool shouldStop();
    void clearTables();
    
    // Time management
    std::chrono::steady_clock::time_point startTime;
    SearchLimits limits;
    bool checkTime();
};
