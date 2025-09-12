#ifndef PVS_H
#define PVS_H

#include "position/position.h"
#include <shared_mutex>

struct SearchResult {
    int score;
    Move bestMove;
    int depth = 0;
};

#include <cstdint>

// zobrist keys
extern uint64_t zobristTable[2][6][8][8]; // [color][pieceType][file][rank]
extern uint64_t zobristSide;              // чей ход
extern uint64_t zobristCastle[16];        // рокировочные права
extern uint64_t zobristEnPassant[8];      // файл взятия на проходе

void initZobrist();
uint64_t computeHash(const Position& pos);

// тип записи в TT
enum class BoundType { EXACT, LOWER, UPPER };

struct TTEntry {
    uint64_t key;
    int depth;
    int score;
    BoundType bound;
    Move bestMove;
};

// сама таблица

class TranspositionTable {
public:
    explicit TranspositionTable(size_t sizeMB = 64);
    bool probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& best);
    void store(uint64_t key, int depth, int score, BoundType bound, const Move& best);

    void clear();
private:
    std::vector<TTEntry> table;
    mutable std::shared_mutex tableMutex;
    size_t sizeMB;
};

int quiescence(Position& pos, int alpha, int beta);
SearchResult pvs(Position& pos, int depth, int alpha, int beta, bool maximizingPlayer, TranspositionTable& tt);

#endif // PVS_H