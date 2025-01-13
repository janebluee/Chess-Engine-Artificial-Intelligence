#include "../../include/mcts/mcts.hpp"
#include <cmath>
#include <algorithm>
#include <chrono>

MCTS::MCTS(std::shared_ptr<Evaluator> eval) : evaluator(eval) {
    root = std::make_unique<Node>();
}

MoveGenerator::Move MCTS::getBestMove(const std::array<uint64_t, 12>& pieces,
                                    uint64_t occupied,
                                    int side,
                                    int castling,
                                    int enPassant,
                                    int timeMs) {
    auto startTime = std::chrono::steady_clock::now();
    int iterations = 0;
    
    #pragma omp parallel
    {
        while (true) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
                         (currentTime - startTime).count();
            
            if (elapsed >= timeMs) {
                break;
            }
            
            #pragma omp critical
            {
                search(root.get(), pieces, occupied, side, 0);
                iterations++;
            }
        }
    }
    
    Node* bestChild = nullptr;
    float bestValue = -std::numeric_limits<float>::infinity();
    
    for (const auto& child : root->children) {
        float childValue = static_cast<float>(child->visits);
        if (childValue > bestValue) {
            bestValue = childValue;
            bestChild = child.get();
        }
    }
    
    return bestChild ? bestChild->move : MoveGenerator::Move{0, 0, 0};
}

void MCTS::search(Node* node, const std::array<uint64_t, 12>& pieces,
                 uint64_t occupied, int side, int depth) {
    if (depth >= 1000) {
        return;
    }
    
    if (node->children.empty()) {
        float value = expand(node, pieces, occupied, side);
        backup(node, value);
        return;
    }
    
    Node* selectedChild = select(node);
    
    #pragma omp atomic
    selectedChild->visits += VIRTUAL_LOSS;
    
    search(selectedChild, pieces, occupied, side ^ 1, depth + 1);
    
    #pragma omp atomic
    selectedChild->visits -= VIRTUAL_LOSS;
}

Node* MCTS::select(Node* node) {
    float bestUct = -std::numeric_limits<float>::infinity();
    Node* selected = nullptr;
    
    float parentVisits = static_cast<float>(node->visits);
    
    for (const auto& child : node->children) {
        float uct = getUCT(child.get(), parentVisits);
        if (uct > bestUct) {
            bestUct = uct;
            selected = child.get();
        }
    }
    
    return selected;
}

float MCTS::expand(Node* node, const std::array<uint64_t, 12>& pieces,
                  uint64_t occupied, int side) {
    float value = evaluator->evaluate(pieces, occupied, side, 0) / 100.0f;
    value = std::tanh(value / 3.0f);
    
    std::vector<MoveGenerator::Move> legalMoves;
    
    for (const auto& child : node->children) {
        legalMoves.push_back(child->move);
    }
    
    float priorSum = 0.0f;
    for (const auto& move : legalMoves) {
        auto childNode = std::make_unique<Node>();
        childNode->move = move;
        childNode->prior = 1.0f / legalMoves.size();
        childNode->parent = node;
        priorSum += childNode->prior;
        node->children.push_back(std::move(childNode));
    }
    
    if (priorSum > 0.0f) {
        for (auto& child : node->children) {
            child->prior /= priorSum;
        }
    }
    
    return -value;
}

void MCTS::backup(Node* node, float value) {
    while (node) {
        #pragma omp atomic
        node->visits++;
        
        #pragma omp atomic
        node->value += value;
        
        value = -value;
        node = node->parent;
    }
}

float MCTS::getUCT(const Node* node, float parentVisits) const {
    float exploitation = node->visits > 0 ? node->value / node->visits : 0.0f;
    float exploration = C_PUCT * node->prior * std::sqrt(parentVisits) / (1 + node->visits);
    return exploitation + exploration;
}
