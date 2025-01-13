#include "../../include/engine/engine.hpp"
#include <algorithm>
#include <thread>

int ChessEngine::alphaBeta(int alpha, int beta, int depth, bool isPV) {
    if (depth <= 0) {
        return quiescence(alpha, beta);
    }

    uint64_t hash = pos.hash();
    TTEntry& entry = transpositionTable[hash & TT_MASK];
    
    if (!isPV && entry.hash == hash && entry.depth >= depth) {
        if (entry.bound == BOUND_EXACT) {
            return entry.score;
        }
        if (entry.bound == BOUND_LOWER && entry.score >= beta) {
            return entry.score;
        }
        if (entry.bound == BOUND_UPPER && entry.score <= alpha) {
            return entry.score;
        }
    }

    std::vector<Move> moves = moveGen->generateLegalMoves(pos.pieces, pos.occupied, pos.side, 
                                                        pos.castling, pos.enPassant);
    
    if (moves.empty()) {
        return isInCheck() ? -MATE_SCORE + pos.ply : 0;
    }

    if (!isPV && !isInCheck()) {
        if (staticEval >= beta + FUTILITY_MARGIN(depth)) {
            return staticEval;
        }
        
        if (canDoNullMove()) {
            makeNullMove();
            int score = -alphaBeta(-beta, -beta + 1, depth - NULL_MOVE_REDUCTION - 1, false);
            unmakeNullMove();
            
            if (score >= beta) {
                return beta;
            }
        }
    }

    Move bestMove;
    int bestScore = -INFINITE;
    int bound = BOUND_UPPER;

    for (size_t i = 0; i < moves.size(); ++i) {
        const Move& move = moves[i];
        
        if (!isPV && !isInCheck() && !isCaptureOrPromotion(move) && 
            staticEval + SEE(move) + FUTILITY_MARGIN(depth) <= alpha) {
            continue;
        }

        makeMove(move);
        
        int score;
        if (i == 0) {
            score = -alphaBeta(-beta, -alpha, depth - 1, isPV);
        } else {
            score = -alphaBeta(-alpha - 1, -alpha, depth - 1, false);
            if (score > alpha && score < beta) {
                score = -alphaBeta(-beta, -alpha, depth - 1, true);
            }
        }
        
        unmakeMove(move);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
            
            if (score > alpha) {
                alpha = score;
                bound = BOUND_EXACT;
                
                if (score >= beta) {
                    updateKillers(move);
                    updateHistory(move, depth);
                    bound = BOUND_LOWER;
                    break;
                }
            }
        }
    }

    entry.hash = hash;
    entry.score = bestScore;
    entry.depth = depth;
    entry.bound = bound;
    entry.bestMove = bestMove;

    return bestScore;
}

int ChessEngine::quiescence(int alpha, int beta) {
    int standPat = evaluator->evaluate(pos.pieces, pos.occupied, pos.side, getGamePhase());
    
    if (standPat >= beta) {
        return beta;
    }
    
    if (standPat > alpha) {
        alpha = standPat;
    }

    std::vector<Move> captures = moveGen->generateCaptures(pos.pieces, pos.occupied, 
                                                         pos.side, pos.enPassant);
    
    for (const Move& move : captures) {
        if (SEE(move) < 0) {
            continue;
        }

        makeMove(move);
        int score = -quiescence(-beta, -alpha);
        unmakeMove(move);

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}
