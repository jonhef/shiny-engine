#pragma once
#include "position/position.h"
#include "../searching/pvs.h"
#include "threading/thread_pool.h"

// Параметры параллельного поиска
struct PvsMtOptions {
    int splitDepth = 3;       // минимальная глубина для распараллеливания братьев
    int minMovesToSplit = 3;  // минимум ходов чтобы распараллеливать
    // additional tunables may be added
};

// Последовательный PVS (используется внутри задач и как fallback)
SearchResult pvs_seq(Position& pos, int depth, int alpha, int beta, TranspositionTable& tt);

// Многопоточный PVS: используется на верхних узлах и рекурсивно вызывает pvs_seq для дочерних.
// pool: готовый ThreadPool, который будет использоваться для запуска задач.
// opts: параметры распараллеливания.
SearchResult pvs_mt_root(Position& pos, int depth, int alpha, int beta,
                         TranspositionTable& tt, ThreadPool& pool, const PvsMtOptions& opts);