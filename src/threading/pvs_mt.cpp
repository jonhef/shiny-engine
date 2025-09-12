#include "pvs_mt.h"

#include <algorithm>
#include <functional>
#include <condition_variable>
#include <chrono>
#include "search_control.h"
#include "evaluation/evaluation.h"

constexpr inline int pieceValue(Figures figure) {
    return figure;
}

// Удобные константы
static constexpr int INF = 1000000000;
static constexpr int MATE = 100000;

// Forward: внутренняя quiescence (negamax-конвенция — score с точки зрения стороны на ходу)
static int quiescence_local(Position& pos, int alpha, int beta);

// ---------------------- Sequential PVS (negamax-style) ----------------------
SearchResult pvs_seq(Position& pos, int depth, int alpha, int beta, TranspositionTable& tt) {
    SearchResult res{};
    res.bestMove = Move{-1,-1,-1,-1, EMPTY};
    res.depth = depth;

    if (search_control::shouldStop()) {
        res.score = 0;
        return res;
    }

    uint64_t key = computeHash(pos);
    int alphaOrig = alpha;

    // TT probe
    int ttScore = 0;
    Move ttMove{-1,-1,-1,-1, EMPTY};
    if (tt.probe(key, depth, alpha, beta, ttScore, ttMove)) {
        res.score = ttScore;
        res.bestMove = ttMove;
        return res;
    }

    if (depth <= 0) {
        res.score = quiescence_local(pos, alpha, beta);
        return res;
    }

    std::vector<Move> moves = pos.getLegalMoves();
    if (moves.empty()) {
        if (pos.isCheck()) res.score = -MATE + depth;
        else res.score = 0;
        return res;
    }

    // ordering: ttMove first
    if (ttMove.fromX != -1) {
        auto it = std::find_if(moves.begin(), moves.end(), [&](const Move& m){
            return m.fromX==ttMove.fromX && m.fromY==ttMove.fromY &&
                   m.toX==ttMove.toX && m.toY==ttMove.toY && m.promotion==ttMove.promotion;
        });
        if (it != moves.end()) std::iter_swap(moves.begin(), it);
    }

    // simple MVV/LVA-like sort for remaining
    std::stable_sort(moves.begin()+1, moves.end(), [&](const Move& a, const Move& b){
        Piece ta = pos.getPiece(a.toX, a.toY);
        Piece tb = pos.getPiece(b.toX, b.toY);
        int va = (ta.getType()==EMPTY && !a.isEnPassant) ? 0 : pieceValue(ta.getType());
        int vb = (tb.getType()==EMPTY && !b.isEnPassant) ? 0 : pieceValue(tb.getType());
        return va > vb;
    });

    int bestScore = -INF;
    Move bestMove = Move{-1,-1,-1,-1, EMPTY};
    bool first = true;

    for (const Move& m : moves) {
        if (search_control::shouldStop()) break;

        Position child = pos;
        child.applyMove(m);

        int val;
        if (first) {
            SearchResult sr = pvs_seq(child, depth-1, -beta, -alpha, tt);
            val = -sr.score;
            first = false;
        } else {
            SearchResult sr = pvs_seq(child, depth-1, -alpha-1, -alpha, tt);
            val = -sr.score;
            if (val > alpha && val < beta) {
                SearchResult sr2 = pvs_seq(child, depth-1, -beta, -alpha, tt);
                val = -sr2.score;
            }
        }

        if (val > bestScore) {
            bestScore = val;
            bestMove = m;
        }
        if (bestScore > alpha) alpha = bestScore;
        if (alpha >= beta) break;
    }

    res.score = bestScore;
    res.bestMove = bestMove;

    // store in TT
    BoundType bound = BoundType::EXACT;
    if (bestScore <= alphaOrig) bound = BoundType::UPPER;
    else if (bestScore >= beta) bound = BoundType::LOWER;
    tt.store(key, depth, res.score, bound, res.bestMove);
    return res;
}

// ---------------------- Local quiescence ----------------------
static int quiescence_local(Position& pos, int alpha, int beta) {
    if (search_control::shouldStop()) return 0;
    int stand = evaluate(pos); // предполагаем, что evaluate уже в negamax-конвенции (side to move)
    // Если evaluate возвращает с точки зрения белых, распакуйте/инвертируйте заранее.
    if (stand >= beta) return beta;
    if (stand > alpha) alpha = stand;

    std::vector<Move> moves = pos.getLegalMoves();
    bool inCheck = pos.isCheck();

    // sort captures first
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b){
        Piece ta = pos.getPiece(a.toX, a.toY);
        Piece tb = pos.getPiece(b.toX, b.toY);
        int va = (ta.getType()==EMPTY && !a.isEnPassant) ? 0 : pieceValue(ta.getType());
        int vb = (tb.getType()==EMPTY && !b.isEnPassant) ? 0 : pieceValue(tb.getType());
        return va > vb;
    });

    for (const Move& m : moves) {
        if (search_control::shouldStop()) break;

        Piece tgt = pos.getPiece(m.toX, m.toY);
        bool isCap = (tgt.getType() != EMPTY) || m.isEnPassant;
        if (!inCheck && !isCap) continue;

        Position child = pos;
        child.applyMove(m);
        int score = -quiescence_local(child, -beta, -alpha);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// ---------------------- Multi-threaded node search (YBWC-style simplified) ----------------------
SearchResult pvs_mt_root(Position& pos, int depth, int alpha, int beta,
                         TranspositionTable& tt, ThreadPool& pool, const PvsMtOptions& opts) {
    SearchResult result{};
    result.bestMove = Move{-1,-1,-1,-1, EMPTY};
    result.depth = depth;

    if (search_control::shouldStop()) {
        result.score = 0;
        return result;
    }

    uint64_t key = computeHash(pos);
    int alphaOrig = alpha;

    // TT probe
    int ttScore = 0;
    Move ttMove{-1,-1,-1,-1, EMPTY};
    if (tt.probe(key, depth, alpha, beta, ttScore, ttMove)) {
        result.score = ttScore;
        result.bestMove = ttMove;
        return result;
    }

    if (depth <= 0) {
        result.score = quiescence_local(pos, alpha, beta);
        return result;
    }

    std::vector<Move> moves = pos.getLegalMoves();
    if (moves.empty()) {
        if (pos.isCheck()) result.score = -MATE + depth;
        else result.score = 0;
        return result;
    }

    // decide to parallelize or not
    if ((int)moves.size() < opts.minMovesToSplit || depth < opts.splitDepth) {
        // fallback to sequential PVS
        return pvs_seq(pos, depth, alpha, beta, tt);
    }

    // order: ttMove first
    if (ttMove.fromX != -1) {
        auto it = std::find_if(moves.begin(), moves.end(), [&](const Move& m){
            return m.fromX==ttMove.fromX && m.fromY==ttMove.fromY &&
                   m.toX==ttMove.toX && m.toY==ttMove.toY && m.promotion==ttMove.promotion;
        });
        if (it != moves.end()) std::iter_swap(moves.begin(), it);
    }

    // first move search in current thread (full window)
    Move bestMove = moves[0];
    Position child0 = pos; child0.applyMove(bestMove);
    SearchResult r0 = pvs_seq(child0, depth-1, -beta, -alpha, tt);
    int bestScore = -r0.score;
    if (bestScore > alpha) alpha = bestScore;
    if (alpha >= beta) {
        result.score = bestScore;
        result.bestMove = bestMove;
        tt.store(key, depth, result.score, BoundType::LOWER, result.bestMove);
        return result;
    }

    // prepare storage for results
    size_t nTasks = moves.size() - 1;
    std::vector<SearchResult> results(nTasks);
    std::vector<std::atomic<bool>> ready(nTasks);
    for (auto &f : ready) f.store(false);
    std::atomic<int> remaining((int)nTasks);
    std::mutex mtx;
    std::condition_variable cv;

    // submit tasks for moves[1..]
    for (size_t i = 0; i < nTasks; ++i) {
        Move mv = moves[i+1];
        Position child = pos; child.applyMove(mv);
        int snapAlpha = alpha;
        TranspositionTable* ttp = &tt;
        // capture child by move to avoid copy
        pool.enqueue([i, child = std::move(child), depth, snapAlpha, ttp, &results, &ready, &remaining, &cv]() mutable {
            if (search_control::shouldStop()) {
                remaining.fetch_sub(1);
                cv.notify_one();
                return;
            }
            // narrow-window search
            SearchResult sr = pvs_seq(const_cast<Position&>(child), depth-1, -snapAlpha-1, -snapAlpha, *ttp);
            results[i] = sr;
            ready[i].store(true);
            remaining.fetch_sub(1);
            cv.notify_one();
        });
    }

    // collect results as they come, and re-search in main thread if necessary
    while (true) {
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait_for(lk, std::chrono::milliseconds(10), [&](){
            return remaining.load() == 0 || search_control::shouldStop();
        });

        if (search_control::shouldStop()) break;

        // scan finished tasks
        for (size_t i = 0; i < nTasks; ++i) {
            if (!ready[i].load()) continue;
            // mark processed
            ready[i].store(false);
            SearchResult sr = results[i];
            int val = -sr.score;
            Move mv = moves[i+1];

            if (val > bestScore) {
                // re-search in main thread with full window
                Position child = pos; child.applyMove(mv);
                SearchResult full = pvs_seq(child, depth-1, -beta, -alpha, tt);
                val = -full.score;
                if (val > bestScore) {
                    bestScore = val;
                    bestMove = mv;
                    if (bestScore > alpha) alpha = bestScore;
                }
            }

            if (alpha >= beta) {
                // beta cutoff, we can stop processing further
                remaining.store(0);
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
