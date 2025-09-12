#include "uci.h"

#include <iostream>
#include <string>
#include "searching/pvs.h"
#include <unordered_map>

void uci_loop() {
    std::string line;
    Position pos;
    TranspositionTable tt(TRANSPOSITIONTABLE_SIZE);
    std::unordered_map<std::string, int> opts;

    while (std::getline(std::cin, line)) {
        if (line == "uci") {
            std::cout << "id name 11yoShiny" << std::endl;
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
        }
        else if (line == "quit") {
            break;
        } else if (line == "copyprotection checking") {
            std::cout << "copyprotection checking" << std::endl;
        } else if (line.rfind("setoption", 0) == 0) {
            
        }
    }
}