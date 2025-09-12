// pvs.cpp
#include "pvs.h"
#include "evaluation/evaluation.h"
#include "position/position.h"
#include <algorithm>

constexpr int INF = 1000000000; // безопасная "бесконечность"

constexpr inline int pieceValue(Figures figure) {
    return figure;
}

// Quiescence: assume evaluate(...) already returns negamax-convention (score from side to move)
int quiescence(Position& pos, int alpha, int beta) {
    int standPat = evaluate(pos);
    // standPat уже в конвенции стороны на ходу
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    std::vector<Move> moves = pos.getLegalMoves();
    bool inCheck = pos.isCheck();

    // simple ordering: captures first
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b){
        Piece ta = pos.getPiece(a.toX, a.toY);
        Piece tb = pos.getPiece(b.toX, b.toY);
        int va = (ta.getType()==EMPTY && !a.isEnPassant) ? 0 : pieceValue(ta.getType());
        int vb = (tb.getType()==EMPTY && !b.isEnPassant) ? 0 : pieceValue(tb.getType());
        return va > vb;
    });

    for (const auto& m : moves) {
        Piece trg = pos.getPiece(m.toX, m.toY);
        bool isCap = (trg.getType() != EMPTY) || m.isEnPassant;
        if (!inCheck && !isCap) continue;

        Position child = pos;
        child.applyMove(m);
        int score = -quiescence(child, -beta, -alpha);
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// PVS (negamax-style)
SearchResult pvs(Position& pos, int depth, int alpha, int beta, bool /*unused*/, TranspositionTable& tt) {
    SearchResult result{};
    result.bestMove = Move{-1,-1,-1,-1, EMPTY};
    result.depth = depth;

    uint64_t key = computeHash(pos);
    int alphaOrig = alpha;

    // TT probe (may provide ordering move even if not usable)
    int ttScore = 0;
    Move ttMove = Move{-1,-1,-1,-1, EMPTY};
    if (tt.probe(key, depth, alpha, beta, ttScore, ttMove)) {
        result.score = ttScore;
        result.bestMove = ttMove;
        return result;
    }

    // leaf
    if (depth <= 0) {
        result.score = quiescence(pos, alpha, beta);
        return result;
    }

    std::vector<Move> moves = pos.getLegalMoves();
    if (moves.empty()) {
        if (pos.isCheck()) {
            result.score = -INF + depth; // mate for side to move
        } else {
            result.score = 0; // stalemate
        }
        return result;
    }

    // move ordering: put TT move first if present
    if (ttMove.fromX != -1) {
        auto it = std::find_if(moves.begin(), moves.end(), [&](const Move& m){
            return m.fromX==ttMove.fromX && m.fromY==ttMove.fromY && m.toX==ttMove.toX && m.toY==ttMove.toY && m.promotion==ttMove.promotion;
        });
        if (it != moves.end()) std::iter_swap(moves.begin(), it);
    }

    // simple ordering: captures next
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

    for (const auto& m : moves) {
        Position child = pos;
        child.applyMove(m);

        int val;
        if (first) {
            SearchResult sr = pvs(child, depth - 1, -beta, -alpha, false, tt);
            val = -sr.score;
            first = false;
        } else {
            SearchResult sr = pvs(child, depth - 1, -alpha - 1, -alpha, false, tt);
            val = -sr.score;
            if (val > alpha && val < beta) {
                SearchResult sr2 = pvs(child, depth - 1, -beta, -alpha, false, tt);
                val = -sr2.score;
            }
        }

        if (val > bestScore) {
            bestScore = val;
            bestMove = m;
        }
        if (bestScore > alpha) alpha = bestScore;
        if (alpha >= beta) break; // beta cutoff
    }

    result.score = bestScore;
    result.bestMove = bestMove;

    // store in TT (use alphaOrig)
    BoundType bound = BoundType::EXACT;
    if (bestScore <= alphaOrig) bound = BoundType::UPPER;
    else if (bestScore >= beta) bound = BoundType::LOWER;
    tt.store(key, depth, bestScore, bound, bestMove);

    return result;
}
