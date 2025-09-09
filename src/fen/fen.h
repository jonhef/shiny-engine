#ifndef FEN_H
#define FEN_H

#include <string>
#include "../position/position.h"

bool decodeFEN(const std::string& fen, Position& pos);
bool encodeFEN(std::string& fen, const Position& pos);

#endif // FEN_H