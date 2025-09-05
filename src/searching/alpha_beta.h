#ifndef ALPHA_BETA_H
#define ALPHA_BETA_H
#include "../utils/position.h"
#include "../utils/chess_logic.h"
#include "tt.h"

#include <random>

using TranspositionTable = std::unordered_map<uint64_t, TTEntry>;

double alpha_beta(
    const Position& pos, 
    int depth, 
    double alpha = std::numeric_limits<double>::min(), 
    double beta = std::numeric_limits<double>::max(), 
    bool maximizing_player = true
);

double pvs(Position& pos, int depth, double alpha, double beta, bool maximizing,
           const Zobrist& zob, TranspositionTable& tt);

Move find_best_move_pvs(const Position& pos, int maxDepth, const Zobrist& zob, TranspositionTable& tt);

#endif // ALPHA_BETA_H