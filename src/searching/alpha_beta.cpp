#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>

#include "alpha_beta.h"
#include "../evaluation/evaluation.h"
#include "../utils/chess_logic.h"

inline double scoreMove(Figures victim, Figures attacker) {
    return victim * 10 - attacker;
}

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
            if (beta <= alpha) break;
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

Move find_best_move(const Position& pos, int maxDepth) {
    Move bestMove;
    double bestEval = std::numeric_limits<double>::min();

    for (int depth = 1; depth <= maxDepth; ++depth) {
        double alpha = std::numeric_limits<double>::min(), beta = std::numeric_limits<double>::max();
        double currentEval = std::numeric_limits<double>::min();
        Move currentBest;

        auto moves = getLegalMoves(pos);

        // Можно отсортировать ходы для alpha-beta
        std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
            return scoreMove(pos.getPieceAt(a.toX, a.toY), pos.getPieceAt(a.fromX, a.fromY)) > scoreMove(pos.getPieceAt(b.toX, b.toY), pos.getPieceAt(b.fromX, b.fromY));
        });

        for (auto& mv : moves) {
            Position after = applyMove(pos, mv);
            double eval = alpha_beta(after, depth - 1, alpha, beta, false);
            if (eval > currentEval) {
                currentEval = eval;
                currentBest = mv;
            }
            alpha = std::max(alpha, eval);
        }

        // После каждой итерации обновляем "лучший ход"
        bestMove = currentBest;
        bestEval = currentEval;
    }

    return bestMove;
}
