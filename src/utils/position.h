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

    bool whiteMove = true;
public:
    Position();
    Position(Position& pos);
    Position(const Position& pos);
    ~Position();

    Board& operator[](Figures figure);
    Board operator[](Figures figure) const;

    Board& operator[](int index);
    Board operator[](int index) const;

    bool getLongWhiteCastling() const;
    bool getShortWhiteCastling() const;
    bool getLongBlackCastling() const;
    bool getShortBlackCastling() const;

    bool& getLongWhiteCastling();
    bool& getShortWhiteCastling();
    bool& getLongBlackCastling();
    bool& getShortBlackCastling();

    bool& isWhiteCastled();
    bool isWhiteCastled() const;
    bool& isBlackCastled();
    bool isBlackCastled() const;

    bool& isWhiteMove();
    bool isWhiteMove() const;

    // NEW: expose en-passant controls
    bool& hasEnPassant();       // reference to 'enPassant' flag
    int& getEnPassantX();       // reference to ep target file (0..7) or -1
    int& getEnPassantY();       // reference to ep target rank (0..7) or -1
    std::pair<int, int> getEnPassantSquare() const;
    void setEnPassantSquare(std::pair<int, int> square);

    bool isTerminal();
};


#endif // POSITION_H