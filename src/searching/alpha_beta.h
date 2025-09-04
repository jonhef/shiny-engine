#include "../utils/position.h"
#include "../utils/chess_logic.h"

int alpha_beta(
    Position pos, 
    int depth, 
    int alpha, 
    int beta, 
    bool maximizing_player
);

Move find_best_move(Position pos, int depth);