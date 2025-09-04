#include <iostream>
#include "utils/position.h"
#include "utils/chess_logic.h"
#include "utils/figures.h"
#include "searching/alpha_beta.h"
#include "utils/fen_utils.h"

int main() {
    Position pos;  // стандартная начальная позиция
    pos.isWhiteMove() = true;

    decodeFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", pos);

    int depth = 4;
    std::cout << "Enter depth: " << std::endl;
    std::cin >> depth;

    int moveCount = 0;

    while(true) {
        auto legalMoves = getLegalMoves(pos);

        if(legalMoves.empty()) {
            if(isCheck(pos)) {
                std::cout << (pos.isWhiteMove() ? "Black" : "White") << " wins by checkmate!\n";
            } else {
                std::cout << "Draw by stalemate!\n";
            }
            break;
        }

        // Выбор лучшего хода через alpha-beta поиск
        Move bestMove = find_best_move(pos, depth);

        // Вывод хода
        std::cout << moveCount + 1 << ". " << (pos.isWhiteMove() ? "White" : "Black")
                  << ": " << bestMove << "\n";

        // Применение хода
        pos = applyMove(pos, bestMove);

        moveCount++;
        if(moveCount >= 200) { // ограничение на число ходов
            std::cout << "Draw by move limit.\n";
            break;
        }
    }

    return 0;
}
