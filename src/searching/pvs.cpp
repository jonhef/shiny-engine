#include "pvs.h"
#include "evaluation/evaluation.h"
#include "position/position.h"

constexpr int INF = 1000000000; // безопасная "бесконечность" для оценок

// --- Quiescence Search ---
int quiescence(Position& pos, int alpha, int beta) {
    int standPat = evaluate(pos);

    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    std::vector<Move> moves = pos.getLegalMoves();

    // только ходы-взятия
    for (auto& m : moves) {
        Piece target = pos.getPiece(m.toX, m.toY);
        if (target.getType() == EMPTY && !m.isEnPassant) continue;

        Position child = pos;
        child.applyMove(m);

        int score = -quiescence(child, -beta, -alpha);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    return alpha;
}

// --- Principal Variation Search ---
SearchResult pvs(Position& pos, int depth, int alpha, int beta, bool maximizingPlayer, TranspositionTable& tt) {
    SearchResult result{};

    uint64_t key = computeHash(pos);
    int ttScore;
    Move ttMove;
    if (tt.probe(key, depth, alpha, beta, ttScore, ttMove)) {
        result.score = ttScore;
        result.bestMove = ttMove;
        return result;
    }

    if (depth == 0) {
        result.score = quiescence(pos, alpha, beta);
        return result;
    }

    std::vector<Move> moves = pos.getLegalMoves();
    if (moves.empty()) {
        if (pos.isCheck()) result.score = maximizingPlayer ? -100000 + depth : 100000 - depth;
        else result.score = 0; // stalemate
        return result;
    }

    bool first = true;
    int bestScore = -INF;
    Move bestMove = moves.front();

    int alphaOrig = alpha; // !!! сохраняем оригинальный alpha для хранения в TT

    for (auto& m : moves) {
        Position child = pos;
        child.applyMove(m);
        int val;

        if (first) {
            SearchResult sr = pvs(child, depth-1, -beta, -alpha, !maximizingPlayer, tt);
            val = -sr.score;
            first = false;
        } else {
            SearchResult sr = pvs(child, depth-1, -alpha-1, -alpha, !maximizingPlayer, tt); // null-window
            val = -sr.score;
            if (val > alpha && val < beta) {
                // re-search with full window
                SearchResult sr2 = pvs(child, depth-1, -beta, -alpha, !maximizingPlayer, tt);
                val = -sr2.score;
            }
        }

        if (val > bestScore) {
            bestScore = val;
            bestMove = m;
        }
        if (bestScore > alpha) alpha = bestScore;
        if (alpha >= beta) break; // beta-cutoff
    }

    result.score = bestScore;
    result.bestMove = bestMove;

    // вычисляем bound, используя неизменённый alphaOrig
    BoundType bound = BoundType::EXACT;
    if (bestScore <= alphaOrig) bound = BoundType::UPPER;
    else if (bestScore >= beta) bound = BoundType::LOWER;
    tt.store(key, depth, bestScore, bound, bestMove);

    return result;
}