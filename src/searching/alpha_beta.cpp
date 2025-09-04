#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>

#include "alpha_beta.h"
#include "../evaluation/evaluation.h"
#include "../utils/chess_logic.h"

double alpha_beta(const Position& pos, int depth, double alpha, double beta, bool maximizing_player) {
    if (depth <= 0 || pos.isTerminal()) return Evaluation::evaluate(pos);

    auto moves = getLegalMoves(pos);
    if (moves.empty()) return Evaluation::evaluate(pos);

    if (maximizing_player) {
        double max_eval = std::numeric_limits<double>::min();
        for (const Move& mv : moves) {
            Position new_pos = applyMove(pos, mv);
            double eval = alpha_beta(new_pos, depth - 1, alpha, beta, false);
            max_eval = std::max(max_eval, eval);
            alpha = std::max(alpha, eval);
            if (alpha >= beta) break;
        }
        return max_eval;
    } else {
        double min_eval = std::numeric_limits<double>::max();
        for (const Move& mv : moves) {
            Position new_pos = applyMove(pos, mv);
            double eval = alpha_beta(new_pos, depth - 1, alpha, beta, true);
            min_eval = std::min(min_eval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

Move find_best_move(const Position& pos, int depth) {
    Move best_move;
    double best_value = std::numeric_limits<double>::min();

    auto moves = getLegalMoves(pos);
    if (moves.empty()) return best_move;

    for (const Move& mv : moves) {
        Position new_pos = applyMove(pos, mv);
        double val = alpha_beta(new_pos, depth - 1,
                             std::numeric_limits<double>::min(),
                             std::numeric_limits<double>::max(),
                             false);
        if (val > best_value) {
            best_value = val;
            best_move = mv;
        }
    }

    return best_move;
}
