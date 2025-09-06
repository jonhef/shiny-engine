// alpha_beta.cpp — Fast PVS with TT, LMR, Killers, History, Aspiration
#include <algorithm>
#include <array>
#include <cfloat>
#include <cstdint>
#include <limits>
#include <unordered_map>
#include <vector>

#include "alpha_beta.h"
#include "../evaluation/evaluation.h"
#include "../utils/chess_logic.h"

using TranspositionTable = std::unordered_map<uint64_t, TTEntry>;

static constexpr double INF = std::numeric_limits<double>::infinity();
static constexpr double NEG_INF = -INF;

// ----------------------------- Helpers ---------------------------------------
inline bool is_capture_or_promo(const Position& pos, const Move& mv) {
    return mv.flag == EN_PASSANT || mv.promoteTo != (Figures)-1 ||
           pos.getPieceAt(mv.toX, mv.toY) != (Figures)-1;
}

inline int square_index(int x, int y) { return y * 8 + x; }

inline int mvv_lva_score(const Position& pos, const Move& mv) {
    Figures victim = pos.getPieceAt(mv.toX, mv.toY);
    Figures attacker = pos.getPieceAt(mv.fromX, mv.fromY);
    if (victim == (Figures)-1 || attacker == (Figures)-1) return 0;
    int v = static_cast<int>(victim);
    int a = static_cast<int>(attacker);
    return (v << 8) - a; // bigger victim, smaller attacker — higher score
}

// ---------------------------- Search context ---------------------------------
struct SearchContext {
    const Zobrist& zob;
    TranspositionTable& tt;
    // Two killer moves per ply
    std::array<std::array<Move, 2>, 128> killers{}; // ply < 128
    // History heuristic by side and from-to
    std::array<std::array<int, 64*64>, 2> history{}; // [side][from*64+to]
    int maxDepth = 0;
    uint64_t nodes = 0;
};

static inline int side_index(const Position& pos) { return pos.isWhiteMove() ? 0 : 1; }

// Update killers: keep unique and non-captures only
static inline void push_killer(SearchContext& sc, int ply, const Move& mv) {
    if (ply >= (int)sc.killers.size()) return;
    if (sc.killers[ply][0] == mv) return;
    sc.killers[ply][1] = sc.killers[ply][0];
    sc.killers[ply][0] = mv;
}

static inline void add_history(SearchContext& sc, const Position& pos, const Move& mv, int depth) {
    if (is_capture_or_promo(pos, mv)) return; // only quiets
    int idx = square_index(mv.fromX, mv.fromY) * 64 + square_index(mv.toX, mv.toY);
    int s = side_index(pos);
    sc.history[s][idx] += depth * depth; // quadratic bump
    if (sc.history[s][idx] > 1'000'000) sc.history[s][idx] /= 2; // avoid overflow
}

static inline int history_score(const SearchContext& sc, const Position& pos, const Move& mv) {
    int idx = square_index(mv.fromX, mv.fromY) * 64 + square_index(mv.toX, mv.toY);
    return sc.history[side_index(pos)][idx];
}

std::vector<Move> generateCaptures(const Position& pos) {
    std::vector<Move> moves;
    std::vector<Move> captures;

    // Сначала получаем все ходы (включая тихие)
    moves = getLegalMoves(pos);

    for (const auto& mv : moves) {
        Figures moving = pos.getPieceAt(mv.fromX, mv.fromY);   // фигура, которая ходит
        Figures target = pos.getPieceAt(mv.toX, mv.toY);     // фигура, стоящая в целевой клетке

        // Взятие: если в target есть фигура соперника
        bool isCapture = (target != EMPTY && ((target > 0 && moving < 0) || (target < 0 && moving > 0)));

        // Превращение пешки: дошла до последней горизонтали
        bool isPromotion = ((moving == WHITE_PAWN || moving == BLACK_PAWN) &&
                           ((moving > 0 && mv.toX == 7) ||
                            (moving < 0 && mv.toX == 0)));

        if (isCapture || isPromotion) {
            captures.push_back(mv);
        }
    }

    return captures;
}

// ----------------------------- Quiescence ------------------------------------
double quiescence(Position& pos, double alpha, double beta, int ply, SearchContext& ctx) {
    ctx.nodes++;  // учитываем узел

    double standPat = Evaluation::evaluate(pos);

    if (standPat >= beta)
        return beta;
    if (standPat > alpha)
        alpha = standPat;

    auto captures = generateCaptures(pos);

    for (const auto& mv : captures) {
        Position next = applyMove(pos, mv);

        double score = -quiescence(next, -beta, -alpha, ply + 1, ctx);

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

// ------------------------------ PVS + LMR ------------------------------------
double search(Position& pos, int depth, double alpha, double beta, int ply,
              SearchContext& sc) {
    ++sc.nodes;

    if (depth <= 0) return quiescence(pos, alpha, beta, ply, sc);

    std::vector<Move> moves;
    moves = getLegalMoves(pos);

    if (moves.empty()) {
        // терминальная позиция: мат или пат
        if (isCheck(pos)) {
            // мат: величина типа -MATE + ply, чтобы предпочитать более быстрый мат
            return -1000000 + ply; 
        } else {
            // пат
            return 0.0;
        }
    }

    // TT probe
    uint64_t key = sc.zob.computeHash(pos);
    auto it = sc.tt.find(key);
    if (it != sc.tt.end()) {
        const TTEntry& e = it->second;
        if (e.depth >= depth) {
            if (e.flag == TTEntry::EXACT) return e.value;
            if (e.flag == TTEntry::LOWERBOUND) alpha = std::max(alpha, e.value);
            else if (e.flag == TTEntry::UPPERBOUND) beta = std::min(beta, e.value);
            if (alpha >= beta) return e.value;
        }
    }

    auto legal = getLegalMoves(pos);
    if (legal.empty()) return Evaluation::evaluate(pos);

    // Move ordering: TT move, captures (MVV/LVA), killers, history
    Move ttMove;
    bool haveTTMove = false;
    if (it != sc.tt.end()) { ttMove = it->second.bestMove; haveTTMove = true; }

    struct Scored { int s; Move m; };
    std::vector<Scored> list; list.reserve(legal.size());

    for (const auto& mv : legal) {
        int score = 0;
        if (haveTTMove && mv == ttMove) score = 1'000'000; // TT on top
        else if (is_capture_or_promo(pos, mv)) score = 500'000 + mvv_lva_score(pos, mv);
        else {
            // killers
            if (ply < (int)sc.killers.size() && (mv == sc.killers[ply][0] || mv == sc.killers[ply][1]))
                score = 200'000;
            score += history_score(sc, pos, mv);
        }
        list.push_back({score, mv});
    }

    std::sort(list.begin(), list.end(), [](const Scored& a, const Scored& b){ return a.s > b.s; });

    bool firstMove = true;
    double best = NEG_INF;
    Move bestMove{};
    double originalAlpha = alpha;

    int moveCount = 0;

    for (const auto& sm : list) {
        const Move& mv = sm.m;
        Position after = applyMove(pos, mv);
        ++moveCount;

        double score;
        if (firstMove) {
            // Full window on the first move — principal variation
            score = -search(after, depth - 1, -beta, -alpha, ply + 1, sc);
            firstMove = false;
        } else {
            // Late Move Reductions for quiet late moves
            int reduction = 0;
            bool quiet = !is_capture_or_promo(pos, mv);
            if (quiet && depth >= 3 && moveCount > 3) {
                // Simple LMR formula
                reduction = 1 + (moveCount > 8);
                // Encourage deeper search for historically good moves
                if (history_score(sc, pos, mv) > 10'000) reduction = std::max(0, reduction - 1);
            }

            int newDepth = std::max(1, depth - 1 - reduction);
            // Null-window (PVS) search
            score = -search(after, newDepth, -alpha - 1e-6, -alpha, ply + 1, sc);
            // Re-search if it improves alpha (fail-high in null-window)
            if (score > alpha && reduction > 0) {
                score = -search(after, depth - 1, -alpha - 1e-6, -alpha, ply + 1, sc);
            }
            if (score > alpha && score < beta) {
                score = -search(after, depth - 1, -beta, -alpha, ply + 1, sc);
            }
        }

        if (score > best) { best = score; bestMove = mv; }
        if (score > alpha) alpha = score;

        // Beta cutoff
        if (alpha >= beta) {
            // TT store as LOWERBOUND
            TTEntry e; e.value = best; e.depth = depth; e.bestMove = bestMove; e.flag = TTEntry::LOWERBOUND;
            sc.tt[key] = e;
            // Killers / history (only on quiets)
            if (!is_capture_or_promo(pos, mv)) {
                push_killer(sc, ply, mv);
                add_history(sc, pos, mv, depth);
            }
            return best;
        }
    }

    // Store TT
    TTEntry e; e.value = best; e.depth = depth; e.bestMove = bestMove;
    e.flag = (best <= originalAlpha) ? TTEntry::UPPERBOUND : TTEntry::EXACT;
    sc.tt[key] = e;

    // Reward PV quiet moves
    if (bestMove.fromX || bestMove.fromY || bestMove.toX || bestMove.toY) {
        if (!is_capture_or_promo(pos, bestMove)) add_history(sc, pos, bestMove, depth);
    }

    return best;
}

// -------------------------- Iterative deepening -------------------------------
Move find_best_move_pvs(const Position& rootPos, int maxDepth, const Zobrist& zob, TranspositionTable& tt) {
    Position pos = rootPos;
    SearchContext sc{zob, tt};
    sc.maxDepth = maxDepth;

    Move bestMove{};
    double bestScore = 0.0; // eval of the PV

    // aspiration window parameters (in centipawns)
    double window = 50.0;

    for (int depth = 1; depth <= maxDepth; ++depth) {
        double alpha = bestScore - window;
        double beta  = bestScore + window;

        // Root move ordering
        auto rootMoves = getLegalMoves(pos);
        if (rootMoves.empty()) break;

        // Use previous TT best move to front
        uint64_t rootKey = zob.computeHash(pos);
        auto it = tt.find(rootKey);
        if (it != tt.end()) {
            auto f = std::find(rootMoves.begin(), rootMoves.end(), it->second.bestMove);
            if (f != rootMoves.end()) std::iter_swap(rootMoves.begin(), f);
        }

        // Score root moves
        struct RootScored { int s; Move m; };
        std::vector<RootScored> rlist; rlist.reserve(rootMoves.size());
        for (const auto& mv : rootMoves) {
            int s = (it != tt.end() && mv == it->second.bestMove) ? 1'000'000 : 0;
            if (is_capture_or_promo(pos, mv)) s += 500'000 + mvv_lva_score(pos, mv);
            s += history_score(sc, pos, mv);
            rlist.push_back({s, mv});
        }
        std::sort(rlist.begin(), rlist.end(), [](const RootScored& a, const RootScored& b){ return a.s > b.s; });

        Move bestAtDepth{}; double bestVal = NEG_INF; bool first = true;

        for (;;) {
            // Search all moves with current (possibly aspirated) window
            double localAlpha = alpha, localBeta = beta;
            bestVal = NEG_INF; first = true; bestAtDepth = Move{};

            for (const auto& rm : rlist) {
                Position after = applyMove(pos, rm.m);
                double score;
                if (first) {
                    score = -search(after, depth - 1, -localBeta, -localAlpha, 1, sc);
                    first = false;
                } else {
                    score = -search(after, depth - 1, -localAlpha - 1e-6, -localAlpha, 1, sc);
                    if (score > localAlpha && score < localBeta)
                        score = -search(after, depth - 1, -localBeta, -localAlpha, 1, sc);
                }
                if (score > bestVal) { bestVal = score; bestAtDepth = rm.m; }
                if (score > localAlpha) localAlpha = score;
            }

            // Aspiration adjustment
            if (bestVal <= alpha) {
                // fail-low: widen down
                alpha -= window; beta = beta; window *= 2.0; // expand only downward first
                if (alpha < -1e9) { alpha = -INF; break; }
                continue;
            }
            if (bestVal >= beta) {
                // fail-high: widen up
                beta += window; window *= 2.0;
                if (beta > 1e9) { beta = INF; break; }
                continue;
            }
            break; // inside window
        }

        bestMove = bestAtDepth;
        bestScore = bestVal;

        // Save principal move at root to TT
        TTEntry rootE; rootE.value = bestScore; rootE.depth = depth; rootE.bestMove = bestMove; rootE.flag = TTEntry::EXACT;
        tt[zob.computeHash(pos)] = rootE;

        // Early exit if a decisive score is found
        if (bestScore > 1e8 || bestScore < -1e8) break;

        // Slowly increase aspiration window for next iteration
        window = std::max(30.0, window * 0.75);
    }

    return bestMove;
}

// -------------------------- Fixed depth evaluation ---------------------------
double evaluate_with_depth(const Position& rootPos, int depth,
                           const Zobrist& zob, TranspositionTable& tt) {
    Position pos = rootPos;
    SearchContext sc{zob, tt};
    sc.maxDepth = depth;

    // Run plain alpha-beta search at exact depth without iterative deepening
    double val = search(pos, depth, -INF, INF, 0, sc);
    return val;
}