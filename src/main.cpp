#include <iostream>

#include "searching/alpha_beta.h"
#include "utils/position.h"
#include "utils/fen_utils.h"
#include "utils/chess_logic.h"

int main(int argc, char* argv[]) {
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Position initial;
    decodeFEN(fen, initial);

    int search_depth = 5;

    Move best_move = find_best_move(initial, search_depth);

    std::cout << best_move;
}