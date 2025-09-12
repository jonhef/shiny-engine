#pragma once
#include "searching/pvs.h"            // SearchResult, pvs signature, TranspositionTable
#include "../position/position.h"
#include "../searching/pvs.h"
#include <thread>
#include "thread_pool.h"

struct ParallelOptions {
    int threads = std::thread::hardware_concurrency();
    int timeMillis = 1000;
    int splitDepth = 3;   // минимальная глубина, с которой распараллеливаем братьев
    int minMovesToSplit = 3; // минимум ходов в узле для split
};

class ParallelSearch {
public:
    ParallelSearch();
    ~ParallelSearch();

    // Запуск параллельного поиска: возвращает лучший результат (bestMove, score, depth)
    SearchResult search(Position root, const ParallelOptions& opts, TranspositionTable& tt);

private:
    // internal worker pool (unique per ParallelSearch instance)
    std::unique_ptr<ThreadPool> pool;

    // запускаем параллельный поиск для узла: обёртка над pvs,
    // реализует распараллеливание братьев на уровне текущего узла.
    SearchResult searchNodeParallel(Position& pos, int depth, int alpha, int beta, TranspositionTable& tt, const ParallelOptions& opts);

    // вспомогательная: полноценный синхронный поиск одного дочернего узла (внутри задачи)
    SearchResult searchChildTask(Position child, int depth, int alpha, int beta, TranspositionTable& tt);

    // Переменные контроля времени
    std::atomic<bool> stopFlag{false};
};
