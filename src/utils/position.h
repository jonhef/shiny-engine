#ifndef POSITION_H
#define POSITION_H

#include <string>
#include <vector>

#include "board.h"
#include "figures.h"

class Position {
    Board white_pawns;
    Board white_knights;
    Board white_bishops;
    Board white_rooks;
    Board white_queens;
    Board white_kings;

    Board black_pawns;
    Board black_knights;
    Board black_bishops;
    Board black_rooks;
    Board black_queens;
    Board black_kings;

    bool castling[4];
    bool enPassant;
public:
    Position();
    Position(Position& pos);
    ~Position();

    Board& operator[](Figures figure);
    Board operator[](Figures figure) const;

    Board& operator[](int index);
    Board operator[](int index) const;
};


#endif // POSITION_H