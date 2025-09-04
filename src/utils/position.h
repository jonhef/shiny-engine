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

    bool longWhiteCastling = false;
    bool shortWhiteCastling = false;

    bool longBlackCastling = false;
    bool shortBlackCastling = false;

    bool whiteCastled = false;
    bool blackCastled = false;

    bool enPassant = false;
    int enPassantX = -1;
    int enPassantY = -1;

    bool whiteMove = false;
public:
    Position();
    Position(Position& pos);
    ~Position();

    Board& operator[](Figures figure);
    Board operator[](Figures figure) const;

    Board& operator[](int index);
    Board operator[](int index) const;

    bool& getLongWhiteCastling();
    bool& getShortWhiteCastling();
    bool& getLongBlackCastling();
    bool& getShortBlackCastling();

    bool& isWhiteCastled();
    bool& isBlackCastled();

    bool& isWhiteMove();
};


#endif // POSITION_H