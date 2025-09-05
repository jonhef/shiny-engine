#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>

#include "alpha_beta.h"
#include "../evaluation/evaluation.h"
#include "../utils/chess_logic.h"

using TranspositionTable = std::unordered_map<uint64_t, TTEntry>;

inline double scoreMove(Figures victim, Figures attacker) {
    return victim * 10 - attacker;
}

double alpha_beta(const Position& pos, int depth, double alpha, double beta, bool maximizing_player) {
    if (depth <= 0 || pos.isTerminal()) return Evaluation::evaluate(pos);

    auto moves = getLegalMoves(pos);
    if (moves.empty()) return Evaluation::evaluate(pos);

    if (maximizing_player) {
        double max_eval = std::numeric_limits<double>::min();
        for (const Move& mv : moves) {
            Position new_pos = applyMove(pos, mv);
            double eval = alpha_beta(new_pos, depth - 1, alpha, beta, false);
            max_eval = std::max(max_eval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return max_eval;
    } else {
        double min_eval = std::numeric_limits<double>::max();
        for (const Move& mv : moves) {
            Position new_pos = applyMove(pos, mv);
            double eval = alpha_beta(new_pos, depth - 1, alpha, beta, true);
            min_eval = std::min(min_eval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return min_eval;
    }
}

double pvs(Position& pos, int depth, double alpha, double beta, bool maximizing, 
           const Zobrist& zob, const TranspositionTable& tt) 
{
    uint64_t hash = zob.computeHash(pos);

    // --- Проверяем TT
    auto it = tt.find(hash);
    if (it != tt.end() && it->second.depth >= depth) {
        const TTEntry& entry = it->second;
        if (entry.flag == TTEntry::EXACT) return entry.value;
        else if (entry.flag == TTEntry::LOWERBOUND && entry.value > alpha) alpha = entry.value;
        else if (entry.flag == TTEntry::UPPERBOUND && entry.value < beta)  beta = entry.value;
        if (alpha >= beta) return entry.value;
    }

    // --- База
    if (depth == 0 || pos.isTerminal()) {
        return Evaluation::evaluate(pos);
    }

    auto moves = getLegalMoves(pos);
    if (moves.empty()) {
        return Evaluation::evaluate(pos); // мат/пат
    }

    bool firstMove = true;
    double bestEval = maximizing ? -1e9 : 1e9;
    Move bestMove;

    // --- Move ordering: TT move в начало
    if (it != tt.end()) {
        auto ttMove = it->second.bestMove;
        auto f = std::find(moves.begin(), moves.end(), ttMove);
        if (f != moves.end()) {
            std::iter_swap(moves.begin(), f);
        }
    }

    for (auto& mv : moves) {
        Position after = applyMove(pos, mv);
        double eval;

        if (firstMove) {
            eval = pvs(after, depth - 1, alpha, beta, !maximizing, zob, tt);
            firstMove = false;
        } else {
            if (maximizing) {
                eval = pvs(after, depth - 1, alpha, alpha + 1, false, zob, tt);
                if (eval > alpha && eval < beta) {
                    eval = pvs(after, depth - 1, alpha, beta, false, zob, tt);
                }
            } else {
                eval = pvs(after, depth - 1, beta - 1, beta, true, zob, tt);
                if (eval < beta && eval > alpha) {
                    eval = pvs(after, depth - 1, alpha, beta, true, zob, tt);
                }
            }
        }

        if (maximizing) {
            if (eval > bestEval) {
                bestEval = eval;
                bestMove = mv;
            }
            alpha = std::max(alpha, eval);
        } else {
            if (eval < bestEval) {
                bestEval = eval;
                bestMove = mv;
            }
            beta = std::min(beta, eval);
        }

        if (alpha >= beta) break;
    }

    // --- Запись в TT
    TTEntry entry;
    entry.value = bestEval;
    entry.depth = depth;
    entry.bestMove = bestMove;

    if (bestEval <= alpha) entry.flag = TTEntry::UPPERBOUND;
    else if (bestEval >= beta) entry.flag = TTEntry::LOWERBOUND;
    else entry.flag = TTEntry::EXACT;

    // ⚠️ TT передаётся const&, поэтому запись нужно делать в вызывающей функции
    // Тут просто возвращаем bestEval, а запись в TT делается выше по стеку

    return bestEval;
}

Move find_best_move(const Position& pos, int maxDepth) {
    Move bestMove;
    double bestEval = std::numeric_limits<double>::min();

    for (int depth = 1; depth <= maxDepth; ++depth) {
        double alpha = std::numeric_limits<double>::min(), beta = std::numeric_limits<double>::max();
        double currentEval = std::numeric_limits<double>::min();
        Move currentBest;

        auto moves = getLegalMoves(pos);

        // Можно отсортировать ходы для alpha-beta
        std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
            return scoreMove(pos.getPieceAt(a.toX, a.toY), pos.getPieceAt(a.fromX, a.fromY)) > scoreMove(pos.getPieceAt(b.toX, b.toY), pos.getPieceAt(b.fromX, b.fromY));
        });

        for (auto& mv : moves) {
            Position after = applyMove(pos, mv);
            double eval = alpha_beta(after, depth - 1, alpha, beta, false);
            if (eval > currentEval) {
                currentEval = eval;
                currentBest = mv;
            }
            alpha = std::max(alpha, eval);
        }

        // После каждой итерации обновляем "лучший ход"
        bestMove = currentBest;
        bestEval = currentEval;
    }

    return bestMove;
}

Move find_best_move_pvs(const Position& pos, int maxDepth, const Zobrist& zob, const TranspositionTable& tt) {
    Move bestMove;
    double bestEval = -std::numeric_limits<double>::infinity();

    for (int depth = 1; depth <= maxDepth; ++depth) {
        double alpha = -std::numeric_limits<double>::infinity();
        double beta  =  std::numeric_limits<double>::infinity();
        Move currentBest;
        double currentEval = -std::numeric_limits<double>::infinity();

        auto moves = getLegalMoves(pos);

        // Сортировка ходов (например, побитовые захваты + history heuristic)
        std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
            return scoreMove(pos.getPieceAt(a.toX, a.toY), pos.getPieceAt(a.fromX, a.fromY)) >
                   scoreMove(pos.getPieceAt(b.toX, b.toY), pos.getPieceAt(b.fromX, b.fromY));
        });

        bool firstMove = true;
        for (auto& mv : moves) {
            Position after = applyMove(pos, mv);

            double eval;
            if (firstMove) {
                // Полный поиск для первого хода (PV node)
                eval = pvs(after, depth - 1, alpha, beta, false, zob, tt);
                firstMove = false;
            } else {
                // Null-window search для остальных ходов
                eval = pvs(after, depth - 1, alpha, alpha + 1e-6, false, zob, tt);
                if (eval > alpha && eval < beta) {
                    // Re-search, если превысило alpha
                    eval = pvs(after, depth - 1, alpha, beta, false, zob, tt);
                }
            }

            if (eval > currentEval) {
                currentEval = eval;
                currentBest = mv;
            }
            alpha = std::max(alpha, eval);
        }

        bestMove = currentBest;
        bestEval = currentEval;
    }

    return bestMove;
}