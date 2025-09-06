#ifndef ALPHA_BETA_H
#define ALPHA_BETA_H
#include "../utils/position.h"
#include "../utils/chess_logic.h"
#include "tt.h"

#include <random>

// Forward declaration; only referenced by const& here
class Zobrist;

using TranspositionTable = std::unordered_map<uint64_t, TTEntry>;

struct SearchContext;

// Compute evaluation (centipawns) of a position with a fixed search depth.
// This runs the PVS alpha-beta search but discards the best move, returning the
// numeric score instead. Positive = advantage to White, negative = advantage to Black.
double evaluate_with_depth(const Position& rootPos, int depth,
                           const Zobrist& zob, TranspositionTable& tt);

double search(Position& pos, int depth, double alpha, double beta, int ply,
              SearchContext& sc);

Move find_best_move_pvs(const Position& pos, int maxDepth, const Zobrist& zob, TranspositionTable& tt);

#endif // ALPHA_BETA_H