#include "evaluation.h"
#include "../utils/ratio.h"
#include "../utils/figures.h"
#include "../utils/board.h"
#include "../utils/position.h"

double Evaluation::evaluate(Position pos) {
    double result = 0;

    result += Ratio(WHITE_PAWN) * pos;
    result += Ratio(WHITE_KNIGHT) * pos;
    result += Ratio(WHITE_BISHOP) * pos;
    result += Ratio(WHITE_ROOK) * pos;
    result += Ratio(WHITE_QUEEN) * pos;
    result += Ratio(WHITE_KING) * pos;

    result -= Ratio(BLACK_PAWN) * pos;
    result -= Ratio(BLACK_KNIGHT) * pos;
    result -= Ratio(BLACK_BISHOP) * pos;
    result -= Ratio(BLACK_ROOK) * pos;
    result -= Ratio(BLACK_QUEEN) * pos;
    result -= Ratio(BLACK_KING) * pos;

    // Penalty for double white pawns
    int countDoublePawns = 0;
    for (int i = 0; i < 8; ++i) {
        bool pawnThere = false;
        for (int j = 1; j < 8; ++j) {
            if (pos[WHITE_PAWN][j][i]) {
                if (pawnThere) {
                    countDoublePawns++;
                    continue;
                }
                pawnThere = true;
            }
        }
    }
    result -= countDoublePawns * PENALTY_DOUBLE_PAWNS;

    // Penalty for double black pawns
    countDoublePawns = 0;
    for (int i = 0; i < 8; ++i) {
        bool pawnThere = false;
        for (int j = 1; j < 8; ++j) {
            if (pos[BLACK_PAWN][j][i]) {
                if (pawnThere) {
                    countDoublePawns++;
                    continue;
                }
                pawnThere = true;
            }
        }
    }
    result += countDoublePawns * PENALTY_DOUBLE_PAWNS;

    // Penalty for isolated white pawns
    int isolatedPawns = 0, isTherePawn = 0b00000000;
    for (int i = 0; i < 8; ++i) {
        for (int j = 1; j < 8; ++j) {
            if (pos[WHITE_PAWN][j][i]) {
                isTherePawn ^= (1 << i);
            }
        }
    }
    
    for (isTherePawn <<= 1; isTherePawn != 0; isTherePawn >>= 1) {
        if ((isTherePawn & 0b111) == 0b010) {
            ++isolatedPawns;
        }
    }

    result -= isolatedPawns * PENALTY_ISOLATED_PAWNS;

    // Penalty for isolated black pawns
    isolatedPawns = 0, isTherePawn = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 1; j < 8; ++j) {
            if (pos[BLACK_PAWN][j][i]) {
                isTherePawn ^= (1 << i);
            }
        }
    }

    for (isTherePawn <<= 1; isTherePawn != 0; isTherePawn >>= 1) {
        if ((isTherePawn & 0b111) == 0b010) {
            ++isolatedPawns;
        }
    }

    result += isolatedPawns * PENALTY_ISOLATED_PAWNS;

    // Penalty for not castled while it is possible for white
    if (!pos.isWhiteCastled() && (pos.getLongWhiteCastling() || pos.getShortWhiteCastling())) {
        result -= PENALTY_NOT_CASTLED_IF_POSSIBLE;
    }

    // Penalty for not castled while it is possible for black
    if (!pos.isBlackCastled() && (pos.getLongBlackCastling() || pos.getShortBlackCastling())) {
        result += PENALTY_NOT_CASTLED_IF_POSSIBLE;
    }

    // Penalty for not castled at all for white
    if (!pos.isWhiteCastled() && !pos.getLongWhiteCastling() && !pos.getShortWhiteCastling()) {
        result -= PENALTY_NOT_CASTLED;
    }

    // Penalty for not castled at all for black
    if (!pos.isBlackCastled() && !pos.getLongBlackCastling() && !pos.getShortBlackCastling()) {
        result += PENALTY_NOT_CASTLED;
    }

    

    return result;
}