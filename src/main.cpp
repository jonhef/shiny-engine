#include <iostream>
#include "position/position.h"
#include "fen/fen.h"
#include "searching/searching.h"
#include "searching/pvs.h"

int main(int argc, char* argv[]) {
    Position pos;
    std::string fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout << "Enter fen: ";
    std::getline(std::cin, fen);

    decodeFEN(fen, pos);

    int time, increment;
    std::cout << "Enter time in format 3+2(3 minutes+2 seconds per move): ";
    {
        std::string stime, sinc;
        std::getline(std::cin, stime, '+');
        std::getline(std::cin, sinc);

        time = std::stoi(stime);
        increment = std::stoi(sinc);
    }

    time *= 60;
    
    initZobrist();
    TranspositionTable tt(128);

    SearchResult res = iterativeDeepeningTime(pos, computeTimeForMove(time, increment, estimateMovesToGo(pos)), tt);
    std::cout << "Evaluation: " << res.score << std::endl;
    std::cout << "Move: (" << res.bestMove.fromX << "," << res.bestMove.fromY
          << ") -> (" << res.bestMove.toX << "," << res.bestMove.toY << ")\n";

    return 0;
}