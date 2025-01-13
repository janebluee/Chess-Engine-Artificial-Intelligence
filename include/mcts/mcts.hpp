#pragma once

#include <memory>
#include <vector>
#include "../utils/move_generator.hpp"
#include "../eval/evaluator.hpp"

class MCTS {
public:
    struct Node {
        MoveGenerator::Move move;
        float value;
        float prior;
        int visits;
        std::vector<std::unique_ptr<Node>> children;
        Node* parent;
    };

    MCTS(std::shared_ptr<Evaluator> eval);
    
    MoveGenerator::Move getBestMove(const std::array<uint64_t, 12>& pieces,
                                  uint64_t occupied,
                                  int side,
                                  int castling,
                                  int enPassant,
                                  int timeMs);
                                  
private:
    static constexpr float C_PUCT = 1.41f;
    static constexpr int VIRTUAL_LOSS = 3;
    
    std::shared_ptr<Evaluator> evaluator;
    std::unique_ptr<Node> root;
    
    void search(Node* node, const std::array<uint64_t, 12>& pieces,
               uint64_t occupied, int side, int depth);
    Node* select(Node* node);
    float expand(Node* node, const std::array<uint64_t, 12>& pieces,
                uint64_t occupied, int side);
    void backup(Node* node, float value);
    float getUCT(const Node* node, float parentVisits) const;
};
