#include "pvs.h"

TranspositionTable::TranspositionTable(size_t sizeMB) {
    size_t entries = (sizeMB * 1024 * 1024) / sizeof(TTEntry);
    table.resize(entries);
}

bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& best) {
    TTEntry& e = table[key % table.size()];
    if (e.key == key && e.depth >= depth) {
        best = e.bestMove;
        switch (e.bound) {
            case BoundType::EXACT: score = e.score; return true;
            case BoundType::LOWER: if (e.score >= beta) { score = e.score; return true; } break;
            case BoundType::UPPER: if (e.score <= alpha) { score = e.score; return true; } break;
        }
    }
    return false;
}

void TranspositionTable::store(uint64_t key, int depth, int score, BoundType bound, const Move& best) {
    TTEntry& e = table[key % table.size()];
    e = { key, depth, score, bound, best };
}