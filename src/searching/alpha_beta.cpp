// alpha_beta_tt_advanced.cpp
#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <unordered_map>
#include <cfloat>

#include "alpha_beta.h"
#include "../evaluation/evaluation.h"
#include "../utils/chess_logic.h"

using TranspositionTable = std::unordered_map<uint64_t, TTEntry>;

static constexpr double INF = std::numeric_limits<double>::infinity();
static constexpr double NEG_INF = -INF;

// ---- MVV/LVA ----
inline int mvv_lva_score(const Position& pos, const Move& mv) {
    Figures victim = pos.getPieceAt(mv.toX, mv.toY);
    Figures attacker = pos.getPieceAt(mv.fromX, mv.fromY);
    if (victim == (Figures)-1 || attacker == (Figures)-1) return 0;
    int v = static_cast<int>(victim);
    int a = static_cast<int>(attacker);
    return (v << 8) - a;
}

// ---- Quiescence ----
double quiescence(const Position& pos, double alpha, double beta, int maxDepth=1) {
    if (maxDepth <= 0) return Evaluation::evaluate(pos);

    double stand_pat = Evaluation::evaluate(pos);
    if (stand_pat >= beta) return stand_pat;
    if (alpha < stand_pat) alpha = stand_pat;

    auto moves = generatePseudoMoves(pos);
    std::vector<Move> captures;
    for (auto &mv : moves) {
        if (mv.flag == EN_PASSANT || mv.promoteTo != (Figures)-1 || pos.getPieceAt(mv.toX, mv.toY) != (Figures)-1)
            captures.push_back(mv);
    }

    if (captures.empty()) return stand_pat;

    std::sort(captures.begin(), captures.end(), [&](const Move &a, const Move &b){
        return mvv_lva_score(pos, a) > mvv_lva_score(pos, b);
    });

    for (const Move &mv : captures) {
        Position after = applyMove(pos, mv);
        double score = -quiescence(after, -beta, -alpha, maxDepth - 1);
        if (score >= beta) return score;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// ---- PVS с TT + TT-based history ----
double pvs(Position& pos, int depth, double alpha, double beta, bool maximizing,
           const Zobrist& zob, TranspositionTable& tt) 
{
    if (depth <= 0) return quiescence(pos, alpha, beta);
    if (pos.isTerminal()) return Evaluation::evaluate(pos);

    uint64_t hash = zob.computeHash(pos);
    auto it = tt.find(hash);

    if (it != tt.end()) {
        const TTEntry& e = it->second;
        if (e.depth >= depth) {
            if (e.flag == TTEntry::EXACT) return e.value;
            if (e.flag == TTEntry::LOWERBOUND && e.value > alpha) alpha = e.value;
            if (e.flag == TTEntry::UPPERBOUND && e.value < beta) beta = e.value;
            if (alpha >= beta) return e.value;
        }
    }

    auto moves = getLegalMoves(pos);
    if (moves.empty()) return Evaluation::evaluate(pos);

    // TT move first
    if (it != tt.end()) {
        auto f = std::find(moves.begin(), moves.end(), it->second.bestMove);
        if (f != moves.end()) std::iter_swap(moves.begin(), f);
    }

    // Сортировка с TT-based history: captures high, затем успешные TT ходы
    std::vector<std::pair<int, Move>> scored;
    scored.reserve(moves.size());
    for (auto &mv : moves) {
        int score = 0;
        Figures victim = pos.getPieceAt(mv.toX, mv.toY);
        if (victim != (Figures)-1 || mv.flag == EN_PASSANT || mv.promoteTo != (Figures)-1)
            score = 100000 + mvv_lva_score(pos, mv);
        else if (it != tt.end() && mv == it->second.bestMove)
            score = 50000 + it->second.historyCount; // TT history
        scored.emplace_back(score, mv);
    }

    std::sort(scored.begin(), scored.end(), [](const auto &a, const auto &b){ return a.first > b.first; });

    bool firstMove = true;
    double bestValue = NEG_INF;
    Move bestMove;
    double localAlpha = alpha;

    for (const auto &pair : scored) {
        const Move &mv = pair.second;
        Position after = applyMove(pos, mv);

        double score;
        if (firstMove) {
            score = -pvs(after, depth - 1, -beta, -localAlpha, !maximizing, zob, tt);
            firstMove = false;
        } else {
            double scoreProbe = -pvs(after, depth - 1, -localAlpha - 1e-6, -localAlpha, !maximizing, zob, tt);
            if (scoreProbe > localAlpha && scoreProbe < beta)
                score = -pvs(after, depth - 1, -beta, -localAlpha, !maximizing, zob, tt);
            else score = scoreProbe;
        }

        if (score > bestValue) {
            bestValue = score;
            bestMove = mv;
        }
        if (score > localAlpha) localAlpha = score;

        if (localAlpha >= beta) {
            TTEntry entry;
            entry.value = bestValue;
            entry.depth = depth;
            entry.bestMove = bestMove;
            entry.flag = TTEntry::LOWERBOUND;
            entry.historyCount = (it != tt.end() ? it->second.historyCount : 0) + depth * depth;
            tt[hash] = entry;
            return bestValue;
        }
    }

    TTEntry entry;
    entry.value = bestValue;
    entry.depth = depth;
    entry.bestMove = bestMove;
    entry.flag = TTEntry::EXACT;
    entry.historyCount = (it != tt.end() ? it->second.historyCount : 0);
    tt[hash] = entry;

    return bestValue;
}

// ---- Найти лучший ход с итеративным углублением ----
Move find_best_move_pvs(const Position& rootPos, int maxDepth, const Zobrist& zob, TranspositionTable& tt) {
    Move bestMove;
    double bestEval = NEG_INF;
    Position pos = rootPos;

    for (int depth = 1; depth <= maxDepth; ++depth) {
        double alpha = NEG_INF;
        double beta  =  INF;

        auto moves = getLegalMoves(pos);
        if (moves.empty()) break;

        uint64_t rootHash = zob.computeHash(pos);
        auto it = tt.find(rootHash);
        if (it != tt.end()) {
            auto f = std::find(moves.begin(), moves.end(), it->second.bestMove);
            if (f != moves.end()) std::iter_swap(moves.begin(), f);
        }

        std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
            int sa = mvv_lva_score(pos, a);
            int sb = mvv_lva_score(pos, b);
            if (sa != sb) return sa > sb;
            if (it != tt.end()) {
                if (a == it->second.bestMove) return true;
                if (b == it->second.bestMove) return false;
            }
            return false;
        });

        Move bestThisDepth;
        double bestThisEval = NEG_INF;
        bool first = true;

        for (auto &mv : moves) {
            Position after = applyMove(pos, mv);
            double score;
            if (first) {
                score = -pvs(after, depth - 1, -beta, -alpha, false, zob, tt);
                first = false;
            } else {
                score = -pvs(after, depth - 1, -alpha - 1e-6, -alpha, false, zob, tt);
                if (score > alpha && score < beta)
                    score = -pvs(after, depth - 1, -beta, -alpha, false, zob, tt);
            }

            if (score > bestThisEval) { bestThisEval = score; bestThisDepth = mv; }
            alpha = std::max(alpha, score);
        }

        bestEval = bestThisEval;
        bestMove = bestThisDepth;

        if (bestEval > 1e8 || bestEval < -1e8) break;
    }

    return bestMove;
}
