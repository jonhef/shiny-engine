#include "../utils/position.h"
#include "../utils/chess_logic.h"

double alpha_beta(
    const Position& pos, 
    int depth, 
    double alpha = std::numeric_limits<double>::min(), 
    double beta = std::numeric_limits<double>::max(), 
    bool maximizing_player = true
);

Move find_best_move(const Position& pos, int depth);