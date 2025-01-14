#include "../../include/mcts/mcts.hpp"
#include "../../include/utils/move_generator.hpp"
#include <cmath>
#include <algorithm>
#include <chrono>
#include <thread>
#include <mutex>
#include <random>

MCTS::MCTS(std::shared_ptr<Evaluator> eval) : evaluator(eval) {
    root = std::make_unique<Node>();
}

MoveGenerator::Move MCTS::getBestMove(const std::array<uint64_t, 12>& pieces,
                                    uint64_t occupied,
                                    int side,
                                    int castling,
                                    int enPassant,
                                    int timeMs) {
    const auto startTime = std::chrono::steady_clock::now();
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(numThreads);
    std::atomic<int> iterations{0};
    std::mutex mtx;
    
    auto threadFunc = [&]() {
        std::mt19937 rng(std::random_device{}());
        while (true) {
            auto currentTime = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() >= timeMs) {
                break;
            }
            
            std::unique_lock<std::mutex> lock(mtx);
            search(root.get(), pieces, occupied, side, 0, rng);
            iterations++;
            lock.unlock();
        }
    };
    
    for (int i = 0; i < numThreads; ++i) {
        threads[i] = std::thread(threadFunc);
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
    
    Node* bestChild = nullptr;
    float bestValue = -std::numeric_limits<float>::infinity();
    int totalVisits = 0;
    
    for (const auto& child : root->children) {
        totalVisits += child->visits;
        float temperature = 1.0f;
        float childValue = std::pow(static_cast<float>(child->visits), 1.0f / temperature);
        if (childValue > bestValue) {
            bestValue = childValue;
            bestChild = child.get();
        }
    }
    
    return bestChild ? bestChild->move : MoveGenerator::Move{0, 0, 0};
}

void MCTS::search(Node* node, const std::array<uint64_t, 12>& pieces,
                 uint64_t occupied, int side, int depth, std::mt19937& rng) {
    if (depth >= 1000) return;
    
    if (node->children.empty()) {
        float value = expand(node, pieces, occupied, side, rng);
        backup(node, value);
        return;
    }
    
    Node* selectedChild = select(node, rng);
    selectedChild->visits += VIRTUAL_LOSS;
    
    search(selectedChild, pieces, occupied, side ^ 1, depth + 1, rng);
    
    selectedChild->visits -= VIRTUAL_LOSS;
}

Node* MCTS::select(Node* node, std::mt19937& rng) {
    float maxValue = -std::numeric_limits<float>::infinity();
    std::vector<Node*> bestChildren;
    float parentVisits = static_cast<float>(node->visits + 1);
    float fpu = -0.2f;
    
    for (const auto& child : node->children) {
        float q = child->visits > 0 ? child->value / child->visits : fpu;
        float u = C_PUCT * child->prior * std::sqrt(parentVisits) / (1.0f + child->visits);
        float puct = q + u;
        
        if (puct > maxValue) {
            maxValue = puct;
            bestChildren.clear();
            bestChildren.push_back(child.get());
        } else if (puct == maxValue) {
            bestChildren.push_back(child.get());
        }
    }
    
    if (bestChildren.empty()) return nullptr;
    
    std::uniform_int_distribution<size_t> dist(0, bestChildren.size() - 1);
    return bestChildren[dist(rng)];
}

float MCTS::expand(Node* node, const std::array<uint64_t, 12>& pieces,
                  uint64_t occupied, int side, std::mt19937& rng) {
    float value = std::tanh(evaluator->evaluate(pieces, occupied, side, 0) / 300.0f);
    
    MoveGenerator moveGen;
    std::vector<MoveGenerator::Move> legalMoves = 
        moveGen.generateLegalMoves(pieces, occupied, side, 0, -1);
    
    if (legalMoves.empty()) {
        return moveGen.isAttacked(pieces[side * 6 + 5], !side, occupied) ? -1.0f : 0.0f;
    }
    
    float priorSum = 0.0f;
    std::uniform_real_distribution<float> noiseDist(0.0f, 1.0f);
    
    for (const auto& move : legalMoves) {
        auto childNode = std::make_unique<Node>();
        childNode->move = move;
        childNode->prior = (1.0f + noiseDist(rng)) / legalMoves.size();
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
    float discount = 1.0f;
    while (node) {
        node->visits++;
        node->value += value * discount;
        value = -value;
        discount *= 0.99f;
        node = node->parent;
    }
}
