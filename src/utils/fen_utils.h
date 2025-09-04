#ifndef DECODE_H
#define DECODE_H

#include <string>
#include <vector>

#include "position.h"

char pieceToChar(int piece);
int charToPiece(char c);

int getPieceAt(const Position& pos, int row, int col);
void setPieceAt(Position& pos, int row, int col, Figures piece);

std::string encodeFEN(Position pos);
void decodeFEN(const std::string &fen, Position);

#endif // DECODE_H