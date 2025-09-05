#ifndef TT_H
#define TT_H
#include <random>
#include <cstdint>

#include "../utils/chess_logic.h"

struct TTEntry {
    double value;
    int depth;
    enum Flag { EXACT, LOWERBOUND, UPPERBOUND } flag;
    Move bestMove;
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