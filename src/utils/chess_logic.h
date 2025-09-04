#ifndef CHESS_LOGIC_H
#define CHESS_LOGIC_H

#include "position.h"
#include <vector>

// Coordinate system:
// White pawns move "up" (y+1), black pawns move "down" (y-1).

enum Flag {
    NONE = 0,
    CAPTURE = 1 << 0,
    EN_PASSANT = 1 << 1,
    CASTLE_SHORT = 1 << 2,
    CASTLE_LONG = 1 << 3,
    DOUBLE_PAWN_PUSH = 1 << 4,
    PROMOTION = 1 << 5
};

struct Move {
    int fromX, fromY;
    int toX, toY;

    // Promotion piece (Figures) for pawn promotion.
    // Use WHITE_QUEEN / WHITE_ROOK / WHITE_BISHOP / WHITE_KNIGHT or BLACK_* accordingly.
    // If no promotion, set to -1.
    int promoteTo;

    // Flags to disambiguate special moves.
    Flag flag;

    Move(int fx=0,int fy=0,int tx=0,int ty=0,int promo=-1, Flag fl=NONE)
        : fromX(fx), fromY(fy), toX(tx), toY(ty), promoteTo(promo), flag(fl) {}
};

// Core APIs you asked for:
bool isCheck(const Position& pos, bool whiteToMove);
bool isCheckmate(const Position& pos, bool whiteToMove);
std::vector<Move> getLegalMoves(const Position& pos, bool whiteToMove);

// (Optionally useful for your engine/UI)
std::vector<Move> generatePseudoMoves(const Position& pos, bool whiteToMove);
Position applyMove(const Position& pos, const Move& mv, bool whiteToMove);

#endif // CHESS_LOGIC_H