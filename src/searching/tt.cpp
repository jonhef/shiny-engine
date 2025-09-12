#include "pvs.h"
#include <cstdint>
#include <shared_mutex>

// --- probe/store с улучшенным поведением и минимальной replacement-логикой ---

TranspositionTable::TranspositionTable(size_t sizeMB) {
    size_t entries = (sizeMB * 1024ULL * 1024ULL) / sizeof(TTEntry);
    if (entries == 0) entries = 1;
    // округлим до ближайшего степеня 2 (опционально) — но не обязательно
    table.resize(entries);
    // инициализируем поля явно (на всякий случай)
    for (auto &e : table) {
        e.key = 0;
        e.depth = -1;
        e.score = 0;
        e.bound = BoundType::EXACT;
        e.bestMove = Move{-1,-1,-1,-1, EMPTY};
    }

    this->sizeMB = sizeMB;
}

bool TranspositionTable::probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& best) {
    size_t idx = key % table.size();
    TTEntry& e = table[idx];

    // default: no usable exact/alpha/beta hit
    best = Move{-1,-1,-1,-1, EMPTY};

    if (e.key != key) return false;

    // всегда отдаём stored bestMove (для ordering), даже если depth < requested
    best = e.bestMove;

    // если глубина записи недостаточна — нельзя безопасно использовать оценку
    if (e.depth < depth) return false;
    
    std::shared_lock<std::shared_mutex> lock(tableMutex);

    // имеем запись с достаточной глубиной — применяем границы
    switch (e.bound) {
        case BoundType::EXACT:
            score = e.score;
            return true;
        case BoundType::LOWER:
            if (e.score >= beta) {
                score = e.score;
                return true;
            }
            break;
        case BoundType::UPPER:
            if (e.score <= alpha) {
                score = e.score;
                return true;
            }
            break;
    }
    return false;
}

void TranspositionTable::store(uint64_t key, int depth, int score, BoundType bound, const Move& best) {
    size_t idx = key % table.size();
    TTEntry& e = table[idx];

    std::unique_lock<std::shared_mutex> lock(tableMutex);

    // Replacement policy:
    // - если слот пустой или ключ совпадает — заменяем
    // - если новая глубина >= старая — заменяем (предпочитаем более глубокую)
    // - иначе оставляем старую запись
    if (e.key == 0 || e.key == key || depth >= e.depth) {
        e.key = key;
        e.depth = depth;
        e.score = score;
        e.bound = bound;
        e.bestMove = best;
    }
    // иначе: не перезаписываем глубже/моложе данные
}

void TranspositionTable::clear() {
    size_t entries = (this->sizeMB * 1024ULL * 1024ULL) / sizeof(TTEntry);
    if (entries == 0) entries = 1;
    // округлим до ближайшего степеня 2 (опционально) — но не обязательно
    table.resize(entries);
    // инициализируем поля явно (на всякий случай)
    for (auto &e : table) {
        e.key = 0;
        e.depth = -1;
        e.score = 0;
        e.bound = BoundType::EXACT;
        e.bestMove = Move{-1,-1,-1,-1, EMPTY};
    }
}