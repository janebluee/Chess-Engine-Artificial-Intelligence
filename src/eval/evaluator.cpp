#include "../../include/eval/evaluator.hpp"
#include <immintrin.h>

Evaluator::Evaluator(std::shared_ptr<NeuralNetwork> nn) : network(nn) {}

int Evaluator::evaluate(const std::array<uint64_t, 12>& pieces,
                       uint64_t occupied,
                       int side,
                       int gamePhase) {
    std::array<float, NeuralNetwork::INPUT_SIZE> features = 
        extractFeatures(pieces, occupied, side);
    
    float nnOutput = network->evaluate(features);
    
    int materialScore = calculateMaterialScore(pieces);
    int positionScore = calculatePositionalScore(nnOutput, gamePhase);
    
    int finalScore = materialScore + positionScore;
    
    return side == 0 ? finalScore : -finalScore;
}

std::array<float, NeuralNetwork::INPUT_SIZE> Evaluator::extractFeatures(
    const std::array<uint64_t, 12>& pieces,
    uint64_t occupied,
    int side) {
    
    std::array<float, NeuralNetwork::INPUT_SIZE> features;
    
    #pragma omp parallel for
    for (int square = 0; square < 64; ++square) {
        for (int piece = 0; piece < 12; ++piece) {
            if (pieces[piece] & (1ULL << square)) {
                int perspective = (piece < 6) == (side == 0) ? square : 63 - square;
                features[piece * 64 + perspective] = 1.0f;
            } else {
                features[piece * 64 + square] = 0.0f;
            }
        }
    }
    
    return features;
}

int Evaluator::calculateMaterialScore(const std::array<uint64_t, 12>& pieces) {
    __m256i scores = _mm256_setzero_si256();
    
    const __m256i values = _mm256_setr_epi32(
        PAWN_VALUE, KNIGHT_VALUE, BISHOP_VALUE, ROOK_VALUE, QUEEN_VALUE, 0,
        -PAWN_VALUE, -KNIGHT_VALUE, -BISHOP_VALUE, -ROOK_VALUE, -QUEEN_VALUE, 0
    );
    
    for (int i = 0; i < 12; i += 8) {
        __m256i counts = _mm256_setr_epi32(
            _mm_popcnt_u64(pieces[i]),
            _mm_popcnt_u64(pieces[i+1]),
            _mm_popcnt_u64(pieces[i+2]),
            _mm_popcnt_u64(pieces[i+3]),
            _mm_popcnt_u64(pieces[i+4]),
            _mm_popcnt_u64(pieces[i+5]),
            _mm_popcnt_u64(pieces[i+6]),
            _mm_popcnt_u64(pieces[i+7])
        );
        
        scores = _mm256_add_epi32(scores, _mm256_mullo_epi32(counts, values));
    }
    
    int total = 0;
    int* score_array = reinterpret_cast<int*>(&scores);
    for (int i = 0; i < 8; ++i) {
        total += score_array[i];
    }
    
    return total;
}

int Evaluator::calculatePositionalScore(float nnOutput, int gamePhase) {
    const float middlegameWeight = gamePhase / 24.0f;
    const float endgameWeight = 1.0f - middlegameWeight;
    
    return static_cast<int>(nnOutput * 100.0f * 
           (middlegameWeight + endgameWeight * 1.3f));
}
