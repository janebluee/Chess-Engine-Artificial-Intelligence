#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <memory>
#include <string>
#include <thread>
#include <iostream>
#include <stdexcept>
#include "../utils/move_generator.hpp"
#include "../neural/neural_network.hpp"
#include "../mcts/mcts.hpp"
#include "../eval/evaluator.hpp"

using Bitboard = uint64_t;

class ChessEngine {
public:
    ChessEngine();
    ~ChessEngine() = default;
    
    void init();
    std::string getBestMove(const std::string& command);
    void setPosition(const std::string& command);
    void setMultiPV(int mpv);
    void setThreadCount(int threads);
    void loadNetwork(const std::string& path);
    
private:
    static constexpr int MAX_DEPTH = 100;
    static constexpr int MAX_PLY = 246;
    static constexpr int TT_SIZE = 1024 * 1024 * 128;
    static constexpr int INFINITE = 30000;
    
    struct SearchInfo {
        int depth{0};
        int64_t nodes{0};
        std::vector<MoveGenerator::Move> pv;
        int score{0};
    };
    
    struct Position {
        std::array<Bitboard, 12> pieces{};
        Bitboard occupied{0};
        int side{0};
        int castling{0};
        int enPassant{-1};
        int fiftyMove{0};
        int ply{0};
    };

    struct Move {
        int from;
        int to;
        int promotion;
        
        Move() : from(0), to(0), promotion(0) {}
        Move(int f, int t, int p) : from(f), to(t), promotion(p) {}
    };
    
    Position pos;
    std::shared_ptr<NeuralNetwork> network;
    std::shared_ptr<Evaluator> evaluator;
    std::unique_ptr<MoveGenerator> moveGen;
    std::shared_ptr<MCTS> mcts;
    std::vector<uint64_t> transpositionTable;
    std::vector<std::thread> threadPool;
    bool useNNUE{true};
    
    void initializeTranspositionTable();
    void loadNetworkWeights();
    void setupThreadPool();
    std::string getBestMoveNNUE(const std::vector<MoveGenerator::Move>& moves);
    std::string getBestMoveMCTS(const std::vector<MoveGenerator::Move>& moves);
    void parseTimeControl(const std::string& command);
    void makeMove(const MoveGenerator::Move& move);
    void unmakeMove(const MoveGenerator::Move& move);
    int alphaBeta(int alpha, int beta, int depth, bool isPV);
    int quiescence(int alpha, int beta);
    void updateSearch(const SearchInfo& info);
    std::string moveToString(const MoveGenerator::Move& move) const;
    std::string extractFEN(const std::string& command) const;
    std::string extractMoves(const std::string& command) const;
    void setStartPosition();
    void setPositionFromFEN(const std::string& fen);
    void applyMoves(const std::string& moves);
    int calculateSearchDepth() const;
};
