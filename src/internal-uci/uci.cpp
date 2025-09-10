#include "uci.h"

#include <iostream>
#include <string>
#include "searching/pvs.h"

void uci_loop() {
    std::string line;
    Position pos;
    TranspositionTable tt(TRANSPOSITIONTABLE_SIZE);
    while (std::getline(std::cin, line)) {
        if (line == "uci") {
            std::cout << "id name shiny-engine 1.0" << std::endl;
            std::cout << "id author jonhef" << std::endl;
            std::cout << "uciok" << std::endl;
        }
        else if (line == "isready") {
            std::cout << "readyok" << std::endl;
        }
        else if (line.rfind("go", 0) == 0) {
            handleGo(line, pos, tt);
        } else if (line.rfind("position", 0) == 0) {
            handlePosition(line, pos);
        }
        else if (line.rfind("ucinewgame", 0) == 0) {
            pos = Position();
            tt = TranspositionTable(TRANSPOSITIONTABLE_SIZE);
        }
        else if (line == "quit") {
            break;
        } else {
            std::cout << "uciok" << std::endl;
            std::cout << "readyok" << std::endl;
        }
    }
}