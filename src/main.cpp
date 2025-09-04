#include <iostream>

#include "searching/alpha_beta.h"
#include "utils/position.h"
#include "utils/fen_utils.h"
#include "utils/chess_logic.h"
#include "evaluation/evaluation.h"

int main(int argc, char* argv[]) {
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Position initial;
    decodeFEN(fen, initial);

    int search_depth = 20;

    std::cout << "Enter depth: ";

    std::cin >> search_depth;
    
    std::cout << encodeFEN(initial) << std::endl;

    std::cout << "Evaluation for initial position: " << (Evaluation::evaluate(initial) / 100) << std::endl;

    Move best_move = find_best_move(initial, search_depth);

    std::cout << "Evaluation for the next best move: " << alpha_beta(applyMove(initial, best_move, false), search_depth) / 100 << std::endl;

    std::cout << best_move << std::endl;
}