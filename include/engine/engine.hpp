#pragma once

#include <array>
#include <vector>
#include <memory>
#include <cstdint>

using Bitboard = uint64_t;

class ChessEngine {
public:
    ChessEngine();
    ~ChessEngine() = default;
    
    void init();
    std::string getBestMove(const std::string& fen, int depth);
    void setMultiPV(int mpv);
    void setThreadCount(int threads);
    void loadNetwork(const std::string& path);
    
private:
    static constexpr int MAX_DEPTH = 100;
    static constexpr int MAX_PLY = 246;
    
    struct SearchInfo {
        int depth;
        int64_t nodes;
        std::vector<uint32_t> pv;
        int score;
    };
    
    struct Position {
        std::array<Bitboard, 12> pieces;
        Bitboard occupied;
        int side;
        int castling;
        int enPassant;
        int fiftyMove;
        int ply;
    };
    
    Position pos;
    std::unique_ptr<class NeuralNetwork> network;
    std::unique_ptr<class MCTS> mcts;
    std::unique_ptr<class Evaluator> evaluator;
    std::unique_ptr<class MoveGenerator> moveGen;
    
    void initPosition(const std::string& fen);
    int alphaBeta(int alpha, int beta, int depth, bool isPV);
    int quiescence(int alpha, int beta);
    void updateSearch(const SearchInfo& info);
};
