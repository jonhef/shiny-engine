#ifndef UCI_H
#define UCI_H

#include <string>
#include "position/position.h"
#include "searching/pvs.h"

constexpr int TRANSPOSITIONTABLE_SIZE = 128; // in megabytes

// helpers
void handlePosition(const std::string& line, Position& pos);
void handleGo(const std::string& line, Position& pos, TranspositionTable& tt);

std::string encodeUCIMove(const Move& mv);

void uci_loop();

#endif // UCI_H