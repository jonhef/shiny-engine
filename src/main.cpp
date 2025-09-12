#include "searching/pvs.h"
#include "searching/searching.h"
#include "position/position.h"
#include "fen/fen.h"
#include <iostream>
#include "internal-uci/uci.h"

int main(int argc, char* argv[]) {
    initZobrist();
    uci_loop();

    return 0;
}