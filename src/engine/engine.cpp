#include "../../include/engine/engine.hpp"
#include <algorithm>
#include <chrono>
#include <sstream>

ChessEngine::ChessEngine() 
    : network(std::make_shared<NeuralNetwork>())
    , moveGen(std::make_unique<MoveGenerator>())
    , evaluator(std::make_shared<Evaluator>(network))
    , mcts(std::make_shared<MCTS>(evaluator))
    , transpositionTable(TT_SIZE)
{
}

void ChessEngine::init() {
    initializeTranspositionTable();
    loadNetworkWeights();
    setupThreadPool();
    setStartPosition();
}

std::string ChessEngine::getBestMove(const std::string& command) {
    parseTimeControl(command);
    
    std::vector<MoveGenerator::Move> legalMoves = 
        moveGen->generateLegalMoves(pos.pieces, pos.occupied, pos.side, pos.castling, pos.enPassant);
        
    if (legalMoves.empty()) {
        return "0000";
    }
    
    return useNNUE ? getBestMoveNNUE(legalMoves) : getBestMoveMCTS(legalMoves);
}

void ChessEngine::setPosition(const std::string& command) {
    std::string fen = extractFEN(command);
    std::string moves = extractMoves(command);
    
    if (fen.empty()) {
        setStartPosition();
    } else {
        setPositionFromFEN(fen);
    }
    
    if (!moves.empty()) {
        applyMoves(moves);
    }
}

void ChessEngine::setMultiPV(int) {
}

void ChessEngine::setThreadCount(int threads) {
    threadPool.resize(threads);
}

void ChessEngine::loadNetwork(const std::string& path) {
    network->loadWeights(path);
}

std::string ChessEngine::getBestMoveNNUE(const std::vector<MoveGenerator::Move>& moves) {
    int bestScore = -INFINITE;
    MoveGenerator::Move bestMove = moves[0];
    
    for (const auto& move : moves) {
        makeMove(move);
        int score = -alphaBeta(-INFINITE, INFINITE, calculateSearchDepth(), true);
        unmakeMove(move);
        
        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }
    
    return moveToString(bestMove);
}

std::string ChessEngine::getBestMoveMCTS(const std::vector<MoveGenerator::Move>&) {
    return moveToString(mcts->getBestMove(pos.pieces, pos.occupied, pos.side, pos.castling, pos.enPassant, 1000));
}

void ChessEngine::parseTimeControl(const std::string&) {
}

void ChessEngine::makeMove(const MoveGenerator::Move& move) {
    Position oldPos = pos;
    
    uint64_t fromBB = 1ULL << move.from;
    uint64_t toBB = 1ULL << move.to;
    
    for (int i = 0; i < 12; ++i) {
        if (pos.pieces[i] & fromBB) {
            pos.pieces[i] &= ~fromBB;
            if (!move.promotion) {
                pos.pieces[i] |= toBB;
            }
            break;
        }
    }
    
    if (move.promotion) {
        pos.pieces[pos.side * 6 + move.promotion - 1] |= toBB;
    }
    
    pos.occupied = 0;
    for (const auto& piece : pos.pieces) {
        pos.occupied |= piece;
    }
    
    pos.side = !pos.side;
    pos.ply++;
}

void ChessEngine::unmakeMove(const MoveGenerator::Move&) {
}

int ChessEngine::alphaBeta(int alpha, int beta, int depth, bool isPV) {
    if (depth <= 0) {
        return quiescence(alpha, beta);
    }
    
    std::vector<MoveGenerator::Move> moves = 
        moveGen->generateLegalMoves(pos.pieces, pos.occupied, pos.side, pos.castling, pos.enPassant);
        
    if (moves.empty()) {
        if (moveGen->isAttacked(pos.pieces[pos.side * 6 + 5], !pos.side, pos.occupied)) {
            return -INFINITE + pos.ply;
        }
        return 0;
    }
    
    for (const auto& move : moves) {
        makeMove(move);
        int score = -alphaBeta(-beta, -alpha, depth - 1, isPV);
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

int ChessEngine::quiescence(int alpha, int beta) {
    int standPat = evaluator->evaluate(pos.pieces, pos.occupied, pos.side, 0);
    
    if (standPat >= beta) {
        return beta;
    }
    
    if (standPat > alpha) {
        alpha = standPat;
    }
    
    std::vector<MoveGenerator::Move> moves = 
        moveGen->generateLegalMoves(pos.pieces, pos.occupied, pos.side, pos.castling, pos.enPassant);
        
    for (const auto& move : moves) {
        uint64_t toBB = 1ULL << move.to;
        if (!(pos.occupied & toBB) && !move.promotion) {
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

void ChessEngine::updateSearch(const SearchInfo&) {
}

std::string ChessEngine::moveToString(const MoveGenerator::Move& move) const {
    static const char* files = "abcdefgh";
    static const char* ranks = "12345678";
    static const char* promotions = "nbrq";
    
    std::string result;
    result += files[move.from & 7];
    result += ranks[move.from >> 3];
    result += files[move.to & 7];
    result += ranks[move.to >> 3];
    
    if (move.promotion) {
        result += promotions[move.promotion - 1];
    }
    
    return result;
}

std::string ChessEngine::extractFEN(const std::string& command) const {
    std::istringstream iss(command);
    std::string token;
    iss >> token;
    
    if (token == "position") {
        iss >> token;
        
        if (token == "startpos") {
            return "";
        }
        
        if (token == "fen") {
            std::string fen;
            for (int i = 0; i < 6; ++i) {
                iss >> token;
                fen += token + " ";
            }
            return fen;
        }
    }
    
    return "";
}

std::string ChessEngine::extractMoves(const std::string& command) const {
    std::istringstream iss(command);
    std::string token;
    
    while (iss >> token) {
        if (token == "moves") {
            std::string moves;
            while (iss >> token) {
                moves += token + " ";
            }
            return moves;
        }
    }
    
    return "";
}

void ChessEngine::setStartPosition() {
    setPositionFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void ChessEngine::setPositionFromFEN(const std::string& fen) {
    std::istringstream iss(fen);
    std::string token;
    
    pos = Position();
    
    iss >> token;
    int rank = 7;
    int file = 0;
    
    for (char c : token) {
        if (c == '/') {
            --rank;
            file = 0;
        } else if (c >= '1' && c <= '8') {
            file += c - '0';
        } else {
            int piece = -1;
            bool isBlack = c >= 'a';
            char p = isBlack ? c : c + 32;
            
            switch (p) {
                case 'p': piece = 0; break;
                case 'n': piece = 1; break;
                case 'b': piece = 2; break;
                case 'r': piece = 3; break;
                case 'q': piece = 4; break;
                case 'k': piece = 5; break;
            }
            
            if (piece != -1) {
                int square = rank * 8 + file;
                pos.pieces[isBlack ? piece + 6 : piece] |= 1ULL << square;
                ++file;
            }
        }
    }
    
    iss >> token;
    pos.side = token == "b";
    
    iss >> token;
    pos.castling = 0;
    for (char c : token) {
        if (c == 'K') pos.castling |= 1;
        if (c == 'Q') pos.castling |= 2;
        if (c == 'k') pos.castling |= 4;
        if (c == 'q') pos.castling |= 8;
    }
    
    iss >> token;
    if (token != "-") {
        pos.enPassant = (token[0] - 'a') + ((token[1] - '1') << 3);
    }
    
    iss >> pos.fiftyMove;
    
    int fullMoves;
    iss >> fullMoves;
    pos.ply = (fullMoves - 1) * 2 + pos.side;
    
    pos.occupied = 0;
    for (const auto& piece : pos.pieces) {
        pos.occupied |= piece;
    }
}

void ChessEngine::applyMoves(const std::string& moves) {
    std::istringstream iss(moves);
    std::string moveStr;
    
    while (iss >> moveStr) {
        MoveGenerator::Move move;
        move.from = (moveStr[0] - 'a') + (moveStr[1] - '1') * 8;
        move.to = (moveStr[2] - 'a') + (moveStr[3] - '1') * 8;
        
        if (moveStr.length() == 5) {
            char p = moveStr[4];
            switch (p) {
                case 'n': move.promotion = 1; break;
                case 'b': move.promotion = 2; break;
                case 'r': move.promotion = 3; break;
                case 'q': move.promotion = 4; break;
            }
        }
        
        makeMove(move);
    }
}

int ChessEngine::calculateSearchDepth() const {
    return 6;
}

void ChessEngine::initializeTranspositionTable() {
    std::fill(transpositionTable.begin(), transpositionTable.end(), 0);
}

void ChessEngine::loadNetworkWeights() {
    const std::string defaultWeightsPath = "weights.bin";
    network->loadWeights(defaultWeightsPath);
}

void ChessEngine::setupThreadPool() {
    threadPool.resize(std::thread::hardware_concurrency());
}
