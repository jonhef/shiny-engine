#ifndef FIGURES_H
#define FIGURES_H

enum Figures {
    EMPTY = 0,
    PAWN = 100,
    KNIGHT = 300,
    BISHOP = 305,
    ROOK = 500,
    QUEEN = 900,
    KING = 400,

    WHITE_PAWN = 100,
    WHITE_KNIGHT = 300,
    WHITE_BISHOP = 305,
    WHITE_ROOK = 500,
    WHITE_QUEEN = 900,
    WHITE_KING = 400,

    BLACK_PAWN = -100,
    BLACK_KNIGHT = -300,
    BLACK_BISHOP = -305,
    BLACK_ROOK = -500,
    BLACK_QUEEN = -900,
    BLACK_KING = -400
};

#endif // FIGURES_H