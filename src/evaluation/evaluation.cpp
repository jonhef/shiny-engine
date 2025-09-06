#include "evaluation.h"
#include "../utils/ratio.h"
#include "../utils/figures.h"
#include "../utils/board.h"
#include "../utils/position.h"
#include "../utils/chess_logic.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <iostream>

// -----------------------------------------------------------------------------
// Classic (pre-NNUE) Stockfish-like hand-crafted evaluation, adapted to the
// available Position API in this project. The code relies only on scanning the
// 8x8 boolean piece grids exposed by Position and avoids engine-specific
// dependencies like move generators. It implements a tapered evaluation with
// MG/EG terms, PSTs, mobility proxies, pawn structure, king safety and a few
// classic bonuses (bishop pair, rooks on open files, tempo).
// -----------------------------------------------------------------------------

namespace {

// Helper aliases
using Score = int; // in centipawns

// Tapered scores encoded as {MG, EG}
struct TScore { Score mg, eg; };
constexpr TScore makeTS(Score mg, Score eg) { return TScore{mg, eg}; }

// Material values (centipawns). These are in the ballpark of classic Stockfish
// pre-NNUE values, slightly simplified.
constexpr Score VAL_P = 100;
constexpr Score VAL_N = 320;
constexpr Score VAL_B = 330;
constexpr Score VAL_R = 500;
constexpr Score VAL_Q = 900;
constexpr Score VAL_K = 0; // king material is not counted in eval score

// Game phase weights (used to compute phase index 0..24 as in Stockfish).
// Heavier pieces contribute more midgame phase.
constexpr int PHASE_N = 1;
constexpr int PHASE_B = 1;
constexpr int PHASE_R = 2;
constexpr int PHASE_Q = 4;
constexpr int PHASE_MAX = 24; // normalization target

// Classic bonuses / penalties
constexpr Score BISHOP_PAIR = 40; // mg
constexpr Score BISHOP_PAIR_EG = 50; // eg
constexpr Score TEMPO_BONUS = 10;

constexpr Score ISOLATED_PAWN_PEN = 15;   // each isolated pawn
constexpr Score DOUBLED_PAWN_PEN = 12;    // each additional pawn on file
constexpr Score BACKWARD_PAWN_PEN = 8;    // crude proxy
constexpr Score PASSED_PAWN_BONUS[8] = { 0, 10, 20, 35, 60, 90, 140, 0 };

constexpr Score ROOK_OPEN_FILE_BONUS = 15;     // no own pawns on file
constexpr Score ROOK_FOPEN_FILE_BONUS = 25;    // completely open file
constexpr Score ROOK_SEMIOPEN_FILE_BONUS = 8;  // no friendly pawns

constexpr Score KNIGHT_OUTPOST = 12; // crude proxy near enemy half and protected by pawn

constexpr Score KING_UNCASTLED_PEN = 20; // extra, on top of header constants

// Pawn shelter (very rough): missing pawns around king
constexpr Score KING_SHELTER_MISSING = 12; // per missing pawn near king

// Piece-Square Tables (MG/EG). Indexed by [row][col] with row 0..7, col 0..7.
// Values are for WHITE; BLACK uses mirrored tables.
using PST = int[8][8];

// A light-weight, smooth PST set (not too swingy) inspired by classical ones.
// Knights love center, bishops like long diagonals, rooks like open files & 7th
// rank (handled separately), queens modest centralization, kings: active in EG.

constexpr PST KNIGHT_MG = {
  { -50,-40,-30,-30,-30,-30,-40,-50 },
  { -40,-20,  0,  0,  0,  0,-20,-40 },
  { -30,  0, 10, 15, 15, 10,  0,-30 },
  { -30,  5, 15, 20, 20, 15,  5,-30 },
  { -30,  0, 15, 20, 20, 15,  0,-30 },
  { -30,  5, 10, 15, 15, 10,  5,-30 },
  { -40,-20,  0,  5,  5,  0,-20,-40 },
  { -50,-40,-30,-30,-30,-30,-40,-50 },
};

constexpr PST KNIGHT_EG = {
  { -40,-30,-20,-20,-20,-20,-30,-40 },
  { -30,-10,  0,  0,  0,  0,-10,-30 },
  { -20,  0, 10, 15, 15, 10,  0,-20 },
  { -20,  0, 15, 20, 20, 15,  0,-20 },
  { -20,  0, 15, 20, 20, 15,  0,-20 },
  { -20,  0, 10, 15, 15, 10,  0,-20 },
  { -30,-10,  0,  0,  0,  0,-10,-30 },
  { -40,-30,-20,-20,-20,-20,-30,-40 },
};

constexpr PST BISHOP_MG = {
  { -20,-10,-10,-10,-10,-10,-10,-20 },
  { -10,  5,  0,  0,  0,  0,  5,-10 },
  { -10, 10, 10, 15, 15, 10, 10,-10 },
  { -10, 10, 15, 20, 20, 15, 10,-10 },
  { -10, 10, 15, 20, 20, 15, 10,-10 },
  { -10, 10, 10, 15, 15, 10, 10,-10 },
  { -10,  5,  0,  0,  0,  0,  5,-10 },
  { -20,-10,-10,-10,-10,-10,-10,-20 },
};

constexpr PST BISHOP_EG = {
  { -15, -5, -5, -5, -5, -5, -5,-15 },
  {  -5, 10,  5,  5,  5,  5, 10, -5 },
  {  -5,  5, 12, 15, 15, 12,  5, -5 },
  {  -5,  5, 15, 20, 20, 15,  5, -5 },
  {  -5,  5, 15, 20, 20, 15,  5, -5 },
  {  -5,  5, 12, 15, 15, 12,  5, -5 },
  {  -5, 10,  5,  5,  5,  5, 10, -5 },
  { -15, -5, -5, -5, -5, -5, -5,-15 },
};

constexpr PST ROOK_MG = {
  { 0, 0, 5, 10, 10, 5, 0, 0 },
  { 0, 0, 5, 10, 10, 5, 0, 0 },
  { 0, 0, 5, 10, 10, 5, 0, 0 },
  { 5, 5, 10, 15, 15,10, 5, 5 },
  { 5, 5, 10, 15, 15,10, 5, 5 },
  { 0, 0, 5, 10, 10, 5, 0, 0 },
  { 0, 0, 5, 10, 10, 5, 0, 0 },
  { 0, 0, 5, 10, 10, 5, 0, 0 },
};

constexpr PST ROOK_EG = {
  { 0, 0, 0, 5, 5, 0, 0, 0 },
  { 0, 0, 0, 5, 5, 0, 0, 0 },
  { 0, 0, 0, 5, 5, 0, 0, 0 },
  { 0, 0, 5,10,10, 5, 0, 0 },
  { 0, 0, 5,10,10, 5, 0, 0 },
  { 0, 0, 0, 5, 5, 0, 0, 0 },
  { 0, 0, 0, 5, 5, 0, 0, 0 },
  { 0, 0, 0, 5, 5, 0, 0, 0 },
};

constexpr PST QUEEN_MG = {
  { -10, -5, -5, -5, -5, -5, -5,-10 },
  {  -5,  0,  0,  0,  0,  0,  0, -5 },
  {  -5,  0,  5,  5,  5,  5,  0, -5 },
  {  -5,  0,  5, 10, 10,  5,  0, -5 },
  {  -5,  0,  5, 10, 10,  5,  0, -5 },
  {  -5,  0,  5,  5,  5,  5,  0, -5 },
  {  -5,  0,  0,  0,  0,  0,  0, -5 },
  { -10, -5, -5, -5, -5, -5, -5,-10 },
};

constexpr PST QUEEN_EG = {
  { -10, -5, -5, -5, -5, -5, -5,-10 },
  {  -5,  0,  0,  0,  0,  0,  0, -5 },
  {  -5,  0,  5,  5,  5,  5,  0, -5 },
  {  -5,  0,  5, 10, 10,  5,  0, -5 },
  {  -5,  0,  5, 10, 10,  5,  0, -5 },
  {  -5,  0,  5,  5,  5,  5,  0, -5 },
  {  -5,  0,  0,  0,  0,  0,  0, -5 },
  { -10, -5, -5, -5, -5, -5, -5,-10 },
};

constexpr PST KING_MG = {
  { 30, 40, 20,  0,  0, 20, 40, 30 },
  { 30, 30,  0, -5, -5,  0, 30, 30 },
  { 10,  0,-10,-15,-15,-10,  0, 10 },
  {  0, -5,-15,-20,-20,-15, -5,  0 },
  {  0, -5,-15,-20,-20,-15, -5,  0 },
  { 10,  0,-10,-15,-15,-10,  0, 10 },
  { 30, 30,  0, -5, -5,  0, 30, 30 },
  { 30, 40, 20,  0,  0, 20, 40, 30 },
};

constexpr PST KING_EG = {
  { -50,-30,-30,-30,-30,-30,-30,-50 },
  { -30,-20,-20,-20,-20,-20,-20,-30 },
  { -30,-20,  0,  0,  0,  0,-20,-30 },
  { -30,-20,  0, 20, 20,  0,-20,-30 },
  { -30,-20,  0, 20, 20,  0,-20,-30 },
  { -30,-20,  0,  0,  0,  0,-20,-30 },
  { -30,-20,-20,-20,-20,-20,-20,-30 },
  { -50,-30,-30,-30,-30,-30,-30,-50 },
};

// Mirror index for BLACK: (row -> 7-row), col unchanged.
inline int mir(int r) { return 7 - r; }

// Utility to check if any piece occupies a square
inline bool occupied(const Position& p, int r, int c) {
    for (int fig = 0; fig < 12; ++fig) { // assuming Figures 0..11 cover all
        if (p[(Figures)fig][r][c]) return true;
    }
    return false;
}

// Count pieces of a given figure and also collect their squares
struct Squares { int count = 0; std::array<std::pair<int,int>, 16> sq; };

inline Squares collect(const Position& p, Figures f) {
    Squares s; s.count = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            if (p[f][r][c]) {
                s.sq[s.count] = {r, c};
                ++s.count;
            }
        }
    }
    return s;
}

inline bool inBounds(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }

// Semi-open/full-open helpers for a file (column)
bool hasPawnOnFile(const Position& p, Figures pawn, int col) {
    for (int r = 0; r < 8; ++r) if (p[pawn][r][col]) return true; return false;
}

bool fileHasAnyPawn(const Position& p, int col) {
    return hasPawnOnFile(p, WHITE_PAWN, col) || hasPawnOnFile(p, BLACK_PAWN, col);
}

// Isolated pawn: no friendly pawns on adjacent files
int countIsolated(const Position& p, Figures pawn) {
    int isolated = 0;
    for (int c = 0; c < 8; ++c) {
        bool left = c > 0 ? hasPawnOnFile(p, pawn, c-1) : false;
        bool right = c < 7 ? hasPawnOnFile(p, pawn, c+1) : false;
        bool hasNeighbor = left || right;
        for (int r = 0; r < 8; ++r) if (p[pawn][r][c] && !hasNeighbor) ++isolated;
    }
    return isolated;
}

// Doubled pawns: additional pawns beyond the first on a file
int countDoubled(const Position& p, Figures pawn) {
    int doubled = 0;
    for (int c = 0; c < 8; ++c) {
        int cnt = 0; for (int r = 0; r < 8; ++r) if (p[pawn][r][c]) ++cnt;
        if (cnt > 1) doubled += (cnt - 1);
    }
    return doubled;
}

// Backward pawn (very crude): pawn with no friendly pawn on adjacent files
// and at least one opposing pawn in front on the same file. Without board
// orientation details, treat "in front" as "anywhere closer to board edge",
// approximated by existence of any opposing pawn on the file.
int countBackwardProxy(const Position& p, Figures pawn, Figures oppPawn) {
    int backw = 0;
    for (int c = 0; c < 8; ++c) {
        bool neighbor = (c>0 && hasPawnOnFile(p, pawn, c-1)) || (c<7 && hasPawnOnFile(p, pawn, c+1));
        bool oppOnFile = hasPawnOnFile(p, oppPawn, c);
        if (!neighbor && oppOnFile) {
            for (int r = 0; r < 8; ++r) if (p[pawn][r][c]) ++backw;
        }
    }
    return backw;
}

// Passed pawn (approximate, orientation-free): no opposing pawns on same or
// adjacent files anywhere on the board.
bool isPassedApprox(const Position& p, int r, int c, Figures ownPawn, Figures oppPawn) {
    for (int dc = -1; dc <= 1; ++dc) {
        int cc = c + dc; if (cc < 0 || cc > 7) continue;
        for (int rr = 0; rr < 8; ++rr) if (p[oppPawn][rr][cc]) return false;
    }
    return true;
}

// Simple pawn shield around king: count friendly pawns within 2 ranks and +/-1
// files of the king square.
int pawnShieldCount(const Position& p, int kr, int kc, Figures pawn) {
    int cnt = 0;
    for (int dr = -2; dr <= 2; ++dr)
        for (int dc = -1; dc <= 1; ++dc) {
            int r = kr + dr, c = kc + dc; if (!inBounds(r,c)) continue;
            if (p[pawn][r][c]) ++cnt;
        }
    return cnt;
}

// Mirror a PST for BLACK (flip rows)
inline int pst(const PST& mg, const PST& eg, bool white, int r, int c, bool endgame) {
    int rr = white ? r : mir(r);
    return endgame ? eg[rr][c] : mg[rr][c];
}

// Compute game phase 0..PHASE_MAX based on remaining material
int gamePhase(const Position& pos) {
    auto wN = collect(pos, WHITE_KNIGHT).count;
    auto wB = collect(pos, WHITE_BISHOP).count;
    auto wR = collect(pos, WHITE_ROOK).count;
    auto wQ = collect(pos, WHITE_QUEEN).count;
    auto bN = collect(pos, BLACK_KNIGHT).count;
    auto bB = collect(pos, BLACK_BISHOP).count;
    auto bR = collect(pos, BLACK_ROOK).count;
    auto bQ = collect(pos, BLACK_QUEEN).count;
    int phase = PHASE_N*(wN+bN) + PHASE_B*(wB+bB) + PHASE_R*(wR+bR) + PHASE_Q*(wQ+bQ);
    return std::min(phase, PHASE_MAX);
}

} // namespace

// -----------------------------------------------------------------------------
// Public entry
// -----------------------------------------------------------------------------

double Evaluation::evaluate(const Position& pos) {
    // Material (side-to-side)
    auto wP = collect(pos, WHITE_PAWN);
    auto wN = collect(pos, WHITE_KNIGHT);
    auto wB = collect(pos, WHITE_BISHOP);
    auto wR = collect(pos, WHITE_ROOK);
    auto wQ = collect(pos, WHITE_QUEEN);
    auto wK = collect(pos, WHITE_KING);

    auto bP = collect(pos, BLACK_PAWN);
    auto bN = collect(pos, BLACK_KNIGHT);
    auto bB = collect(pos, BLACK_BISHOP);
    auto bR = collect(pos, BLACK_ROOK);
    auto bQ = collect(pos, BLACK_QUEEN);
    auto bK = collect(pos, BLACK_KING);

    Score mg = 0, eg = 0;

    // Base material
    Score wMat = wP.count*VAL_P + wN.count*VAL_N + wB.count*VAL_B + wR.count*VAL_R + wQ.count*VAL_Q;
    Score bMat = bP.count*VAL_P + bN.count*VAL_N + bB.count*VAL_B + bR.count*VAL_R + bQ.count*VAL_Q;
    mg += (wMat - bMat);
    eg += (wMat - bMat);

    // Bishop pair
    if (wB.count >= 2) { mg += BISHOP_PAIR; eg += BISHOP_PAIR_EG; }
    if (bB.count >= 2) { mg -= BISHOP_PAIR; eg -= BISHOP_PAIR_EG; }

    // Piece-Square contributions
    auto addPST = [&](bool white, const Squares& sq, const PST& mgT, const PST& egT) {
        for (int i = 0; i < sq.sq.size(); ++i) {
            int r = sq.sq[i].first, c = sq.sq[i].second;
            mg += white ? pst(mgT, egT, true,  r, c, false) : -pst(mgT, egT, false, r, c, false);
            eg += white ? pst(mgT, egT, true,  r, c, true ) : -pst(mgT, egT, false, r, c, true );
        }
    };

    addPST(true,  wN, KNIGHT_MG, KNIGHT_EG);
    addPST(false, bN, KNIGHT_MG, KNIGHT_EG);
    addPST(true,  wB, BISHOP_MG, BISHOP_EG);
    addPST(false, bB, BISHOP_MG, BISHOP_EG);
    addPST(true,  wR, ROOK_MG,   ROOK_EG);
    addPST(false, bR, ROOK_MG,   ROOK_EG);
    addPST(true,  wQ, QUEEN_MG,  QUEEN_EG);
    addPST(false, bQ, QUEEN_MG,  QUEEN_EG);
    addPST(true,  wK, KING_MG,   KING_EG);
    addPST(false, bK, KING_MG,   KING_EG);

    // Pawn structure: isolated, doubled, backward, passed
    int wIsol = countIsolated(pos, WHITE_PAWN);
    int bIsol = countIsolated(pos, BLACK_PAWN);
    mg += -(ISOLATED_PAWN_PEN * wIsol) + (ISOLATED_PAWN_PEN * bIsol);
    eg += -(ISOLATED_PAWN_PEN/2 * wIsol) + (ISOLATED_PAWN_PEN/2 * bIsol);

    int wDoub = countDoubled(pos, WHITE_PAWN);
    int bDoub = countDoubled(pos, BLACK_PAWN);
    mg += -(DOUBLED_PAWN_PEN * wDoub) + (DOUBLED_PAWN_PEN * bDoub);

    int wBack = countBackwardProxy(pos, WHITE_PAWN, BLACK_PAWN);
    int bBack = countBackwardProxy(pos, BLACK_PAWN, WHITE_PAWN);
    mg += -(BACKWARD_PAWN_PEN * wBack) + (BACKWARD_PAWN_PEN * bBack);

    for (int i = 0; i < wP.count; ++i) {
        int r = wP.sq[i].first, c = wP.sq[i].second;
        if (isPassedApprox(pos, r, c, WHITE_PAWN, BLACK_PAWN)) {
            mg += PASSED_PAWN_BONUS[r];
            eg += PASSED_PAWN_BONUS[r] + 10; // a bit more valuable in EG
        }
    }
    for (int i = 0; i < bP.count; ++i) {
        int r = bP.sq[i].first, c = bP.sq[i].second;
        if (isPassedApprox(pos, r, c, BLACK_PAWN, WHITE_PAWN)) {
            mg -= PASSED_PAWN_BONUS[mir(r)];
            eg -= PASSED_PAWN_BONUS[mir(r)] + 10;
        }
    }

    // Rooks on (semi-)open files
    for (int i = 0; i < wR.count; ++i) {
        int c = wR.sq[i].second;
        bool own = hasPawnOnFile(pos, WHITE_PAWN, c);
        bool opp = hasPawnOnFile(pos, BLACK_PAWN, c);
        if (!own && !opp) { mg += ROOK_FOPEN_FILE_BONUS; eg += ROOK_FOPEN_FILE_BONUS; }
        else if (!own && opp) { mg += ROOK_OPEN_FILE_BONUS; eg += ROOK_OPEN_FILE_BONUS/2; }
        else if (!own) { mg += ROOK_SEMIOPEN_FILE_BONUS; }
    }
    for (int i = 0; i < bR.count; ++i) {
        int c = bR.sq[i].second;
        bool own = hasPawnOnFile(pos, BLACK_PAWN, c);
        bool opp = hasPawnOnFile(pos, WHITE_PAWN, c);
        if (!own && !opp) { mg -= ROOK_FOPEN_FILE_BONUS; eg -= ROOK_FOPEN_FILE_BONUS; }
        else if (!own && opp) { mg -= ROOK_OPEN_FILE_BONUS; eg -= ROOK_OPEN_FILE_BONUS/2; }
        else if (!own) { mg -= ROOK_SEMIOPEN_FILE_BONUS; }
    }

    // Knight outposts (proxy): knight protected by pawn and placed on central ranks
    auto knightOutpost = [&](bool white, const Squares& kn, Figures ownPawn) {
        for (int i = 0; i < kn.count; ++i) {
            int r = kn.sq[i].first, c = kn.sq[i].second;
            bool central = (r >= 2 && r <= 5 && c >= 2 && c <= 5);
            bool protectedByPawn = false;
            for (int dc = -1; dc <= 1; dc += 2) {
                int rr = r + (white ? 1 : -1); // approximate support square
                int cc = c + dc;
                if (inBounds(rr,cc) && pos[ownPawn][rr][cc]) { protectedByPawn = true; break; }
            }
            if (central && protectedByPawn) {
                mg += white ? KNIGHT_OUTPOST : -KNIGHT_OUTPOST;
                eg += white ? KNIGHT_OUTPOST/2 : -KNIGHT_OUTPOST/2;
            }
        }
    };
    knightOutpost(true,  wN, WHITE_PAWN);
    knightOutpost(false, bN, BLACK_PAWN);

    // King safety: castling + pawn shelter near king
    if (!pos.isWhiteCastled() && (pos.getShortWhiteCastling() || pos.getLongWhiteCastling())) {
        mg -= PENALTY_NOT_CASTLED_IF_POSSIBLE + KING_UNCASTLED_PEN;
    }
    if (!pos.isBlackCastled() && (pos.getShortBlackCastling() || pos.getLongBlackCastling())) {
        mg += PENALTY_NOT_CASTLED_IF_POSSIBLE + KING_UNCASTLED_PEN;
    }
    if (!pos.isWhiteCastled() && !pos.getShortWhiteCastling() && !pos.getLongWhiteCastling()) {
        mg -= PENALTY_NOT_CASTLED;
    }
    if (!pos.isBlackCastled() && !pos.getShortBlackCastling() && !pos.getLongBlackCastling()) {
        mg += PENALTY_NOT_CASTLED;
    }

    // Pawn shelter around kings
    auto shelter = [&](bool white, const Squares& king, Figures pawn) {
        if (king.count == 0) return; // shouldn't happen
        int kr = king.sq[0].first, kc = king.sq[0].second;
        int have = pawnShieldCount(pos, kr, kc, pawn);
        int missing = std::max(0, 3 - have);
        Score delta = missing * KING_SHELTER_MISSING;
        mg += white ? -delta : +delta;
    };
    shelter(true,  wK, WHITE_PAWN);
    shelter(false, bK, BLACK_PAWN);

    // Tempo bonus to side to move (small but classic)
    if (pos.isWhiteMove()) { mg += TEMPO_BONUS; eg += TEMPO_BONUS; }
    else { mg -= TEMPO_BONUS; eg -= TEMPO_BONUS; }

    // Blend MG/EG by phase
    int phase = gamePhase(pos);
    Score blended = ( (mg * phase) + (eg * (PHASE_MAX - phase)) ) / PHASE_MAX;

    // Checkmate detection (large bound to stabilize search like a win)
    if (isCheckmate(pos)) {
        // If current side to move is checkmated, the previous mover delivered mate
        if (pos.isWhiteMove()) blended -= 1000000; else blended += 1000000;
    }

    return static_cast<double>(blended);
}