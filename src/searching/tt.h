#ifndef TT_H
#define TT_H

#include <random>
#include <cstdint>
#include "../utils/chess_logic.h"

struct TTEntry {
    double value;      // оценка позиции
    int depth;         // глубина поиска, на которой записано значение
    enum Flag { EXACT, LOWERBOUND, UPPERBOUND } flag;
    Move bestMove;     // лучший ход для позиции
    int historyCount;  // счетчик успешных ходов для TT-based history heuristic

    TTEntry() : value(0), depth(0), flag(EXACT), bestMove(), historyCount(0) {}
};

class Zobrist {
    uint64_t zobristTable[12][64]; // 12 фигур × 64 клетки
    uint64_t zobristSide;          // чей ход
    uint64_t zobristCastling[16];  // 16 возможных прав на рокировку (4 бита)
    uint64_t zobristEnPassant[8];  // если есть взятие на проходе
public: 
    Zobrist();
    ~Zobrist();

    uint64_t computeHash(const Position& pos) const;
};

#endif // TT_H