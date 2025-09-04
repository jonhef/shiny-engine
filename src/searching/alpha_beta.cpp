#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>

#include "alpha_beta.h"
#include "../evaluation/evaluation.h"
#include "../utils/chess_logic.h"

int alpha_beta(
    Position pos, 
    int depth, 
    int alpha, 
    int beta, 
    bool maximizing_player
) {
    if (depth == 0 || pos.isTerminal()) {
        return Evaluation::evaluate(pos);
    }

    if (maximizing_player) {
        int max_eval = std::numeric_limits<int>::min();
        for (const Move& move : getLegalMoves(pos, pos.isWhiteMove())) {
            Position new_pos(pos);
            applyMove(new_pos, move, new_pos.isWhiteMove());
            int eval = alpha_beta(new_pos, depth - 1, alpha, beta, false);
            max_eval = std::max(max_eval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return max_eval;
    } else {
        int min_eval = std::numeric_limits<int>::max();
        for (const Move& move : getLegalMoves(pos, pos.isWhiteMove())) {
            Position new_pos(pos);
            int eval = alpha_beta(new_pos, depth - 1, alpha, beta, true);
            min_eval = std::min(min_eval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) {
                break;
            }
        }
        return min_eval;
    }
}

Move find_best_move(Position pos, int depth) {
    Move best_move;
    int best_value = std::numeric_limits<int>::min();

    for (const Move& move : getLegalMoves(pos, pos.isWhiteMove())) {
        Position new_pos = applyMove(pos, move, pos.isWhiteMove());
        int move_value = alpha_beta(new_pos, depth - 1, 
                                  std::numeric_limits<int>::min(),
                                  std::numeric_limits<int>::max(),
                                  false); // Следующий ход - минимизирующего игрока
        
        if (move_value > best_value) {
            best_value = move_value;
            best_move = move;
        }
    }
    
    return best_move;
}
