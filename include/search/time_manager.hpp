#pragma once
#include <chrono>

class TimeManager {
public:
    struct TimeInfo {
        int wtime = -1;    // White's remaining time in milliseconds
        int btime = -1;    // Black's remaining time in milliseconds
        int winc = 0;      // White's increment per move in milliseconds
        int binc = 0;      // Black's increment per move in milliseconds
        int movestogo = 0; // Number of moves to go before next time control
        int depth = -1;    // Maximum search depth
        int nodes = 0;     // Maximum nodes to search
        int mate = 0;      // Search for mate in x moves
        int movetime = -1; // Time to search for this move only
        bool infinite = false; // Search until stopped
    };

    static std::chrono::milliseconds calculateMoveTime(const TimeInfo& info, bool isWhite) {
        if (info.movetime != -1) return std::chrono::milliseconds(info.movetime);
        if (info.infinite) return std::chrono::milliseconds(0);
        
        int time = isWhite ? info.wtime : info.btime;
        int inc = isWhite ? info.winc : info.binc;
        
        if (time == -1) return std::chrono::milliseconds(0);
        
        int moveTime;
        
        if (info.movestogo > 0) {
            // Reserve some time for remaining moves
            moveTime = time / info.movestogo;
            if (inc > 0) {
                moveTime += inc * 3 / 4;
            }
        } else {
            // Estimate we have about 40 moves remaining
            moveTime = time / 40;
            if (inc > 0) {
                moveTime += inc * 3 / 4;
            }
        }
        
        // Never use more than 20% of remaining time
        moveTime = std::min(moveTime, time / 5);
        
        // Always leave at least 100ms on the clock
        moveTime = std::min(moveTime, time - 100);
        
        return std::chrono::milliseconds(moveTime);
    }
};
