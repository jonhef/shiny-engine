#include "evaluation.h"
#include "../utils/ratio.h"
#include "../utils/figures.h"
#include "../utils/board.h"
#include "../utils/position.h"
#include "../utils/chess_logic.h"

double Evaluation::evaluate(Position pos) {
    double result = 0.0;

    // --- Материальные ценности в сантипешках ---
    result += Ratio(WHITE_PAWN) * pos;
    result += Ratio(WHITE_KNIGHT) * pos;
    result += Ratio(WHITE_BISHOP) * pos;
    result += Ratio(WHITE_ROOK) * pos;
    result += Ratio(WHITE_QUEEN) * pos;
    result += Ratio(WHITE_KING) * pos;

    result += Ratio(BLACK_PAWN) * pos;
    result += Ratio(BLACK_KNIGHT) * pos;
    result += Ratio(BLACK_BISHOP) * pos;
    result += Ratio(BLACK_ROOK) * pos;
    result += Ratio(BLACK_QUEEN) * pos;
    result += Ratio(BLACK_KING) * pos;

    // --- Пешки: двойные и изолированные ---
    auto countDoublePawns = [](const Position &p, Figures pawn) {
        int cnt = 0;
        for (int col = 0; col < 8; ++col) {
            bool pawnThere = false;
            for (int row = 0; row < 8; ++row) {
                if (p[pawn][row][col]) {
                    if (pawnThere) cnt++;
                    pawnThere = true;
                }
            }
        }
        return cnt;
    };

    auto countIsolatedPawns = [](const Position &p, Figures pawn) {
        int isolated = 0;
        for (int col = 0; col < 8; ++col) {
            bool left = (col > 0), right = (col < 7);
            bool hasNeighbor = false;
            for (int row = 0; row < 8; ++row) {
                if ((left && p[pawn][row][col-1]) || (right && p[pawn][row][col+1]))
                    hasNeighbor = true;
            }
            for (int row = 0; row < 8; ++row) {
                if (p[pawn][row][col] && !hasNeighbor) isolated++;
            }
        }
        return isolated;
    };

    result -= PENALTY_DOUBLE_PAWNS * countDoublePawns(pos, WHITE_PAWN);
    result += PENALTY_DOUBLE_PAWNS * countDoublePawns(pos, BLACK_PAWN);

    result -= PENALTY_ISOLATED_PAWNS * countIsolatedPawns(pos, WHITE_PAWN);
    result += PENALTY_ISOLATED_PAWNS * countIsolatedPawns(pos, BLACK_PAWN);

    // --- Централизация фигур ---
    auto centralizationBonus = [](const Position &p, Figures piece) {
        double bonus = 0;
        for (int row = 0; row < 8; ++row) {
            for (int col = 0; col < 8; ++col) {
                if (p[piece][row][col]) {
                    // центр: d4, e4, d5, e5
                    int distRow = std::abs(3.5 - row);
                    int distCol = std::abs(3.5 - col);
                    bonus += (4.0 - (distRow + distCol)) * 10.0; // вес 10 сантипешек
                }
            }
        }
        return bonus;
    };

    result += centralizationBonus(pos, WHITE_KNIGHT);
    result += centralizationBonus(pos, WHITE_BISHOP);
    result += centralizationBonus(pos, WHITE_QUEEN);
    result -= centralizationBonus(pos, BLACK_KNIGHT);
    result -= centralizationBonus(pos, BLACK_BISHOP);
    result -= centralizationBonus(pos, BLACK_QUEEN);

    // --- Рокировка и безопасность короля ---
    if (!pos.isWhiteCastled() && (pos.getShortWhiteCastling() || pos.getLongWhiteCastling()))
        result -= PENALTY_NOT_CASTLED_IF_POSSIBLE;
    if (!pos.isBlackCastled() && (pos.getShortBlackCastling() || pos.getLongBlackCastling()))
        result += PENALTY_NOT_CASTLED_IF_POSSIBLE;

    if (!pos.isWhiteCastled() && !pos.getShortWhiteCastling() && !pos.getLongWhiteCastling())
        result -= PENALTY_NOT_CASTLED;
    if (!pos.isBlackCastled() && !pos.getShortBlackCastling() && !pos.getLongBlackCastling())
        result += PENALTY_NOT_CASTLED;

    // --- Ладьи на открытых и полностью открытых линиях ---
    auto rookOpenLineBonus = [](const Position &p, Figures rook, Figures pawnOpp, Figures pawnSelf) {
        int bonus = 0;
        for (int col = 0; col < 8; ++col) {
            bool hasPawnSelf = false, hasPawnOpp = false;
            for (int row = 0; row < 8; ++row) {
                if (p[pawnSelf][row][col]) hasPawnSelf = true;
                if (p[pawnOpp][row][col]) hasPawnOpp = true;
            }
            for (int row = 0; row < 8; ++row) {
                if (p[rook][row][col]) {
                    if (!hasPawnSelf && hasPawnOpp) bonus += REWARD_ROOK_OPEN_TILE;
                    if (!hasPawnSelf && !hasPawnOpp) bonus += REWARD_ROOK_FULLOPEN_TILE;
                }
            }
        }
        return bonus;
    };

    result += rookOpenLineBonus(pos, WHITE_ROOK, BLACK_PAWN, WHITE_PAWN);
    result -= rookOpenLineBonus(pos, BLACK_ROOK, WHITE_PAWN, BLACK_PAWN);

    // --- Проверка мата ---
    if (isCheckmate(pos)) {
        result += 10000 * (pos.isWhiteMove() ? 1 : -1);
    }

    // --- Конечная нормализация ---
    // всё в сантипешках (100 = 1 пешка)
    return result;
}