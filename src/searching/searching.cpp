// Вставь эти include в начало searching.cpp (если ещё нет)
#include "threading/thread_pool.h"
#include "../threading/search_control.h"
#include <atomic>
#include <condition_variable>
#include <chrono>
#include "../threading/pvs_mt.h"
#include "../position/position.h"

// Константы
constexpr int INF_SEARCH = 1000000000;

// ---------- root-parallel по глубине, фиксированное число потоков ----------
SearchResult iterativeDeepeningThreadsDepth(Position& pos, int maxDepth, TranspositionTable& tt, int numThreads) {
    SearchResult globalBest{};
    globalBest.depth = 0;
    globalBest.score = 0;
    globalBest.bestMove = Move{-1,-1,-1,-1, EMPTY};

    // защитные значения
    int hw = std::max(1u, std::thread::hardware_concurrency());
    int nThreads = std::max(1, std::min(numThreads, (int)hw));

    // для каждой глубины запускаем распределение корневых ходов на nThreads
    for (int depth = 1; depth <= maxDepth; ++depth) {
        if (search_control::shouldStop()) break;

        std::vector<Move> rootMoves = pos.getLegalMoves();
        if (rootMoves.empty()) {
            // мат/пат
            if (pos.isCheck()) { globalBest.score = -INF_SEARCH + depth; }
            else { globalBest.score = 0; }
            return globalBest;
        }

        // контейнеры для результатов
        size_t M = rootMoves.size();
        std::vector<SearchResult> results(M);
        std::atomic<size_t> nextIdx{0};

        // ThreadPool локальный для этой глубины (можно переделать на глобальный)
        ThreadPool pool(nThreads);

        // worker: берёт индекс и обрабатывает соответствующий root move
        auto worker = [&](void) {
            while (!search_control::shouldStop()) {
                size_t i = nextIdx.fetch_add(1);
                if (i >= M) break;
                // подготовка child
                Position child = pos;
                child.applyMove(rootMoves[i]);

                // делаем синхронный поиск на child (depth-1). Используем pvs (thread-safe)
                // вызываем с широким окном: -INF..INF
                SearchResult r = pvs(child, depth - 1, -INF_SEARCH, INF_SEARCH, false, tt);
                // сохраняем
                results[i] = r;
            }
        };

        // запустить nThreads воркеров (enqueue nThreads задач, каждый цикл берёт столько задач, сколько нужно)
        for (int t = 0; t < nThreads; ++t) {
            pool.enqueue([worker](){ worker(); });
        }

        // ждать пока все задачи будут выполнены или стоп
        // проверяем завершение по nextIdx
        while (!search_control::shouldStop()) {
            if (nextIdx.load() >= M) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        // попросим пул корректно завершиться (локально)
        pool.shutdown();

        // Если стоп — не принимаем эту глубину (возвращаем последнее подтверждённое)
        if (search_control::shouldStop()) break;

        // выбираем лучший ход по результатам (negamax: score child возвращён с точки зрения стороны на ход у child,
        // поэтому итоговая оценка корневого хода = - child.score)
        int bestScore = -INF_SEARCH;
        Move bestMove = rootMoves[0];
        for (size_t i = 0; i < M; ++i) {
            int sc = -results[i].score;
            if (sc > bestScore) {
                bestScore = sc;
                bestMove = rootMoves[i];
            }
        }

        // обновим глобальный лучший результат
        globalBest.score = bestScore;
        globalBest.bestMove = bestMove;
        globalBest.depth = depth;
    }

    return globalBest;
}

// ---------- root-parallel по времени, фиксированное число потоков ----------
SearchResult iterativeDeepeningThreadsTime(Position& pos, int timeMillis, TranspositionTable& tt, int numThreads) {
    using clock_t = search_control::clock_t;
    SearchResult globalBest{};
    globalBest.depth = 0;
    globalBest.score = 0;
    globalBest.bestMove = Move{-1,-1,-1,-1, EMPTY};

    // защита параметров
    int hw = std::max(1u, std::thread::hardware_concurrency());
    int nThreads = std::max(1, std::min(numThreads, (int)hw));

    // ставим deadline
    search_control::setDeadlineMillis(timeMillis);

    int depth = 1;
    while (!search_control::shouldStop()) {
        std::vector<Move> rootMoves = pos.getLegalMoves();
        if (rootMoves.empty()) {
            if (pos.isCheck()) { globalBest.score = -INF_SEARCH + depth; }
            else { globalBest.score = 0; }
            return globalBest;
        }

        size_t M = rootMoves.size();
        std::vector<std::atomic<bool>> done(M);
        for (auto &b : done) b.store(false);
        std::vector<SearchResult> results(M);

        ThreadPool pool(nThreads);
        std::atomic<size_t> nextIdx{0};
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<int> remaining((int)M);

        // submit worker tasks (each task will take indices until none left)
        for (int t = 0; t < nThreads; ++t) {
            pool.enqueue([&]() {
                while (!search_control::shouldStop()) {
                    size_t i = nextIdx.fetch_add(1);
                    if (i >= M) break;
                    Position child = pos;
                    child.applyMove(rootMoves[i]);

                    SearchResult r = pvs(child, depth - 1, -INF_SEARCH, INF_SEARCH, false, tt);
                    results[i] = r;
                    done[i].store(true);
                    remaining.fetch_sub(1);
                    cv.notify_one();
                }
            });
        }

        // wait until either all done or deadline
        bool depthCompleted = true;
        clock_t::time_point deadline = search_control::deadline;

        {
            std::unique_lock<std::mutex> lk(mtx);
            while (remaining.load() > 0 && !search_control::shouldStop()) {
                if (cv.wait_until(lk, deadline) == std::cv_status::timeout) {
                    // time is up
                    depthCompleted = false;
                    search_control::requestStop();
                    break;
                }
            }
        }

        // shutdown pool (let workers finish)
        pool.shutdown();

        if (!depthCompleted) {
            // не принимаем неполную глубину
            break;
        }

        // все завершили — выбираем лучший
        int bestScore = -INF_SEARCH;
        Move bestMove = rootMoves[0];
        for (size_t i = 0; i < M; ++i) {
            int sc = -results[i].score;
            if (sc > bestScore) {
                bestScore = sc;
                bestMove = rootMoves[i];
            }
        }

        // update global best
        globalBest.score = bestScore;
        globalBest.bestMove = bestMove;
        globalBest.depth = depth;

        // next depth
        ++depth;
    }

    return globalBest;
}

// старые имена (без numThreads) — используют все доступные потоки
SearchResult iterativeDeepeningThreadsDepth(Position& pos, int maxDepth, TranspositionTable& tt) {
    return iterativeDeepeningThreadsDepth(pos, maxDepth, tt, std::max(1u, std::thread::hardware_concurrency()));
}
SearchResult iterativeDeepeningThreadsTime(Position& pos, int timeMillis, TranspositionTable& tt) {
    return iterativeDeepeningThreadsTime(pos, timeMillis, tt, std::max(1u, std::thread::hardware_concurrency()));
}