#ifndef TT_H
#define TT_H

#include <random>
#include <cstdint>
#include "../utils/chess_logic.h"
// Transposition Table entry used by search
struct TTEntry {
    // Evaluated score from the POV of the side to move in 'Position' when stored
    double value = 0.0;
    // Remaining search depth (in plies) the value was computed with
    int    depth = 0;
    // Best move found from this position at the stored depth
    Move   bestMove{};
    // Node type for alpha-beta bounds handling
    enum Flag : uint8_t { EXACT = 0, LOWERBOUND = 1, UPPERBOUND = 2 } flag = EXACT;
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