#pragma once

#include <cstdint>
#include <array>
#include <memory>
#include <vector>
#include <random>
#include "../utils/move_generator.hpp"
#include "../eval/evaluator.hpp"

class MCTS {
public:
    struct Node {
        MoveGenerator::Move move{};
        float value{0.0f};
        float prior{0.0f};
        int visits{0};
        std::vector<std::unique_ptr<Node>> children;
        Node* parent{nullptr};
        
        Node() = default;
        ~Node() = default;
    };

    explicit MCTS(std::shared_ptr<Evaluator> eval);
    ~MCTS() = default;
    
    MoveGenerator::Move getBestMove(const std::array<uint64_t, 12>& pieces,
                                  uint64_t occupied,
                                  int side,
                                  int castling,
                                  int enPassant,
                                  int timeMs) const;
                                  
private:
    static constexpr float C_PUCT = 1.41f;
    static constexpr int VIRTUAL_LOSS = 3;
    
    std::shared_ptr<Evaluator> evaluator;
    std::unique_ptr<Node> root;
    
    void search(Node* node, const std::array<uint64_t, 12>& pieces,
               uint64_t occupied, int side, int depth, std::mt19937& rng) const;
    Node* select(Node* node, std::mt19937& rng) const;
    float expand(Node* node, const std::array<uint64_t, 12>& pieces,
                uint64_t occupied, int side, std::mt19937& rng) const;
    void backup(Node* node, float value) const;
    float getUCT(const Node* node, float parentVisits) const;
};
