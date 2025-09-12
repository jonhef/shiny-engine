#include "parallel_search.h"
#include "thread_pool.h"
#include "../searching/pvs.h"
#include "search_control.h"

ParallelSearch::ParallelSearch() {}
ParallelSearch::~ParallelSearch() {}

SearchResult ParallelSearch::search(Position root, const ParallelOptions& opts, TranspositionTable& tt) {
    pool = std::make_unique<ThreadPool>(opts.threads);
    search_control::setDeadlineMillis(opts.timeMillis);

    // start iterative deepening at root, but each depth we use searchNodeParallel for root.
    SearchResult best{}; best.depth = 0; best.score = -1000000000;
    for (int d = 1; d <= 64; ++d) {
        if (search_control::shouldStop()) break;
        // call parallel node search at root
        SearchResult r = searchNodeParallel(root, d, -1000000000, 1000000000, tt, opts);
        r.depth = d;
        {
            // store best only if fully finished depth (searchNodeParallel fully completes)
            best = r;
        }
        if (search_control::shouldStop()) break;
    }

    pool->shutdown();
    pool.reset();
    return best;
}

SearchResult ParallelSearch::searchChildTask(Position child, int depth, int alpha, int beta, TranspositionTable& tt) {
    // use existing pvs function - it is synchronous and uses TT
    return pvs(child, depth, alpha, beta, false, tt);
}

// Core: parallelized node processing (YBWC-style but using a threadpool).
SearchResult ParallelSearch::searchNodeParallel(Position& pos, int depth, int alpha, int beta, TranspositionTable& tt, const ParallelOptions& opts) {
    SearchResult result{};
    result.bestMove = Move{-1,-1,-1,-1, EMPTY};
    uint64_t key = computeHash(pos);
    int alphaOrig = alpha;

    // quick TT probe
    int ttScore = 0;
    Move ttMove{-1,-1,-1,-1, EMPTY};
    if (tt.probe(key, depth, alpha, beta, ttScore, ttMove)) {
        result.score = ttScore;
        result.bestMove = ttMove;
        return result;
    }

    if (depth <= 0) {
        result.score = quiescence(pos, alpha, beta);
        return result;
    }

    std::vector<Move> moves = pos.getLegalMoves();
    if (moves.empty()) {
        if (pos.isCheck()) result.score = -1000000000 + depth;
        else result.score = 0;
        return result;
    }

    if ((int)moves.size() < opts.minMovesToSplit || depth < opts.splitDepth || opts.threads <= 1) {
        return pvs(pos, depth, alpha, beta, false, tt);
    }

    // ordering: try TT move first
    if (ttMove.fromX != -1) {
        auto it = std::find_if(moves.begin(), moves.end(), [&](const Move& m){
            return m.fromX==ttMove.fromX && m.fromY==ttMove.fromY && m.toX==ttMove.toX && m.toY==ttMove.toY && m.promotion==ttMove.promotion;
        });
        if (it != moves.end()) std::iter_swap(moves.begin(), it);
    }

    // First move in this thread (full window)
    Move bestMove = moves[0];
    Position firstChild = pos; firstChild.applyMove(bestMove);
    SearchResult firstRes = pvs(firstChild, depth-1, -beta, -alpha, false, tt);
    int bestScore = -firstRes.score;
    if (bestScore > alpha) alpha = bestScore;
    if (alpha >= beta) {
        result.score = bestScore;
        result.bestMove = bestMove;
        tt.store(key, depth, result.score, BoundType::LOWER, result.bestMove);
        return result;
    }

    // Prepare concurrent result storage for spawned tasks
    size_t nTasks = moves.size() - 1;
    std::vector<SearchResult> results(nTasks);
    std::vector<std::atomic<bool>> finished(nTasks);
    for (auto &f : finished) f.store(false);
    std::atomic<int> remaining((int)nTasks);
    std::mutex resMutex;
    std::condition_variable resCv;

    // Submit tasks for moves[1..]
    for (size_t i = 0; i < nTasks; ++i) {
        Move mv = moves[i+1];
        Position childPos = pos; childPos.applyMove(mv);
        // snapshot alpha for narrow-window
        int snapAlpha = alpha;

        // Capture by value what we need. tt pointer for shared TT
        TranspositionTable* ttp = &tt;
        pool->enqueue([i, childPos = std::move(childPos), depth, snapAlpha, ttp, &results, &finished, &remaining, &resMutex, &resCv, mv]() mutable {
            if (search_control::shouldStop()) {
                // leave finished false; main thread will break eventually
                remaining.fetch_sub(1);
                resCv.notify_one();
                return;
            }
            // narrow-window search
            SearchResult r = pvs(const_cast<Position&>(childPos), depth-1, -snapAlpha-1, -snapAlpha, false, *ttp);

            // store result
            results[i] = r;
            finished[i].store(true);
            remaining.fetch_sub(1);
            resCv.notify_one();
        });
    }

    // Collect results as they arrive
    for (;;) {
        std::unique_lock<std::mutex> lk(resMutex);
        resCv.wait_for(lk, std::chrono::milliseconds(10), [&](){ return remaining.load() == 0 || search_control::shouldStop(); });

        if (search_control::shouldStop()) break;

        // scan finished results and update best
        for (size_t i = 0; i < nTasks; ++i) {
            if (!finished[i].load()) continue;
            // process and mark processed by setting finished[i] = false to avoid double processing
            finished[i].store(false);
            SearchResult sr = results[i];
            int val = -sr.score;
            Move mv = moves[i+1];

            if (val > bestScore) {
                // re-search in main thread with full window to get exact score
                Position child = pos; child.applyMove(mv);
                SearchResult full = pvs(child, depth-1, -beta, -alpha, false, tt);
                val = -full.score;
                if (val > bestScore) {
                    bestScore = val;
                    bestMove = mv;
                    if (bestScore > alpha) alpha = bestScore;
                }
            }

            if (alpha >= beta) {
                // cutoff, we can cancel other tasks (they will observe shouldStop eventually)
                // We won't actively cancel tasks, but break out and record result
                remaining.store(0); // indicate we're done processing
                break;
            }
        }

        if (remaining.load() == 0) break;
    }

    result.score = bestScore;
    result.bestMove = bestMove;

    BoundType bound = BoundType::EXACT;
    if (bestScore <= alphaOrig) bound = BoundType::UPPER;
    else if (bestScore >= beta) bound = BoundType::LOWER;
    tt.store(key, depth, result.score, bound, result.bestMove);

    return result;
}
