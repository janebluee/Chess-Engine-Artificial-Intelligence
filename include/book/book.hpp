#pragma once
#include "../board/board.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

class OpeningBook {
public:
    struct BookMove {
        uint16_t move;
        int weight;
        int wins;
        int draws;
        int losses;
    };

    OpeningBook();
    ~OpeningBook();

    // Load book from file
    bool loadFromFile(const std::string& filename);
    
    // Save book to file
    bool saveToFile(const std::string& filename) const;
    
    // Get moves for position
    std::vector<BookMove> getMovesForPosition(const Board& board) const;
    
    // Add move to book
    void addMove(const Board& board, uint16_t move, int weight = 1);
    
    // Update statistics
    void updateStats(const Board& board, uint16_t move, int result);
    
    // Clear the book
    void clear();
    
    // Book size
    size_t size() const;

private:
    struct BookEntry {
        std::vector<BookMove> moves;
    };
    
    std::unordered_map<uint64_t, BookEntry> positions;
    
    // Internal helpers
    void addMoveToEntry(BookEntry& entry, uint16_t move, int weight);
    BookMove* findMove(BookEntry& entry, uint16_t move);
};
