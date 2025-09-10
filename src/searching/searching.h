#ifndef SEARCHING_H
#define SEARCHING_H

#include "pvs.h"

SearchResult iterativeDeepeningDepth(Position& pos, int maxDepth, TranspositionTable& tt);
SearchResult iterativeDeepeningTime(Position& pos, int maxMillis, TranspositionTable& tt);

inline int estimateMovesToGo(const Position& pos) {
    int material = 0;
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            Piece p = pos.getPiece(x, y);
            if (p.getType() != EMPTY) material += (int)p.getType();
        }
    }
    return material / 60;             // эндшпиль
}

constexpr inline int computeTimeForMove(int timeLeftMs, int incrementMs, int movesToGo = 30) {
    if (timeLeftMs <= 0) return 50; // fallback

    // базовое распределение
    int timeForMove = timeLeftMs / movesToGo + incrementMs;

    // ограничение
    int minTime = 10;
    int maxTime = timeLeftMs / 2;

    if (timeForMove < minTime) timeForMove = minTime;
    if (timeForMove > maxTime) timeForMove = maxTime;

    return timeForMove;
}


#endif // SEARCHING_H