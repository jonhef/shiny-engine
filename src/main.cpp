#include <iostream>
#include "position/position.h"
#include "fen/fen.h"

int main(int argc, char* argv[]) {
    Position pos;
    std::string fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout << "Enter fen: ";
    // std::getline(std::cin, fen);

    decodeFEN(fen, pos);

    std::cout << fen << std::endl;

    encodeFEN(fen, pos);

    std::cout << fen << std::endl;

    return 0;
}