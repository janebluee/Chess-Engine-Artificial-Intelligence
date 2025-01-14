#include "../../include/book/book.hpp"
#include <fstream>
#include <random>
#include <algorithm>

OpeningBook::OpeningBook() {}
OpeningBook::~OpeningBook() {}

bool OpeningBook::loadFromFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return false;
    
    clear();
    
    // Read number of positions
    size_t numPositions;
    file.read(reinterpret_cast<char*>(&numPositions), sizeof(numPositions));
    
    // Read each position
    for (size_t i = 0; i < numPositions; ++i) {
        uint64_t key;
        size_t numMoves;
        
        file.read(reinterpret_cast<char*>(&key), sizeof(key));
        file.read(reinterpret_cast<char*>(&numMoves), sizeof(numMoves));
        
        BookEntry& entry = positions[key];
        entry.moves.resize(numMoves);
        
        // Read moves for this position
        file.read(reinterpret_cast<char*>(entry.moves.data()), 
                 numMoves * sizeof(BookMove));
    }
    
    return true;
}

bool OpeningBook::saveToFile(const std::string& filename) const {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;
    
    // Write number of positions
    size_t numPositions = positions.size();
    file.write(reinterpret_cast<const char*>(&numPositions), sizeof(numPositions));
    
    // Write each position
    for (const auto& [key, entry] : positions) {
        size_t numMoves = entry.moves.size();
        
        file.write(reinterpret_cast<const char*>(&key), sizeof(key));
        file.write(reinterpret_cast<const char*>(&numMoves), sizeof(numMoves));
        file.write(reinterpret_cast<const char*>(entry.moves.data()),
                  numMoves * sizeof(BookMove));
    }
    
    return true;
}

std::vector<OpeningBook::BookMove> OpeningBook::getMovesForPosition(const Board& board) const {
    uint64_t key = board.getHash();
    auto it = positions.find(key);
    if (it == positions.end()) return {};
    
    return it->second.moves;
}

void OpeningBook::addMove(const Board& board, uint16_t move, int weight) {
    uint64_t key = board.getHash();
    BookEntry& entry = positions[key];
    addMoveToEntry(entry, move, weight);
}

void OpeningBook::updateStats(const Board& board, uint16_t move, int result) {
    uint64_t key = board.getHash();
    auto it = positions.find(key);
    if (it == positions.end()) return;
    
    BookMove* bookMove = findMove(it->second, move);
    if (!bookMove) return;
    
    if (result > 0) bookMove->wins++;
    else if (result < 0) bookMove->losses++;
    else bookMove->draws++;
}

void OpeningBook::clear() {
    positions.clear();
}

size_t OpeningBook::size() const {
    return positions.size();
}

void OpeningBook::addMoveToEntry(BookEntry& entry, uint16_t move, int weight) {
    BookMove* existing = findMove(entry, move);
    if (existing) {
        existing->weight += weight;
        return;
    }
    
    entry.moves.push_back({move, weight, 0, 0, 0});
    std::sort(entry.moves.begin(), entry.moves.end(),
              [](const BookMove& a, const BookMove& b) {
                  return a.weight > b.weight;
              });
}

OpeningBook::BookMove* OpeningBook::findMove(BookEntry& entry, uint16_t move) {
    for (auto& bookMove : entry.moves) {
        if (bookMove.move == move) return &bookMove;
    }
    return nullptr;
}
