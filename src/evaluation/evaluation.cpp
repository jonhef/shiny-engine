#include "evaluation.h"
#include <array>
#include <unordered_map>
#include <algorithm>

/* Helpers */
using PST = std::array<std::array<int, 8>, 8>;

// pawn
constexpr PST pawn_table = {{ 
    {{  0,   0,   0,   0,   0,   0,   0,   0 }},
    {{  5,  10,  10, -20, -20,  10,  10,   5 }},
    {{  5,  -5, -10,   0,   0, -10,  -5,   5 }},
    {{  0,   0,   0,  20,  20,   0,   0,   0 }},
    {{  5,   5,  10,  25,  25,  10,   5,   5 }},
    {{ 10,  10,  20,  30,  30,  20,  10,  10 }},
    {{ 50,  50,  50,  50,  50,  50,  50,  50 }},
    {{  0,   0,   0,   0,   0,   0,   0,   0 }}
}}; 

// knight
constexpr PST knight_table = {{
    {{-50, -40, -30, -30, -30, -30, -40, -50}},
    {{-40, -20,   0,   0,   0,   0, -20, -40}},
    {{-30,   0,  10,  15,  15,  10,   0, -30}},
    {{-30,   5,  15,  20,  20,  15,   5, -30}},
    {{-30,   0,  15,  20,  20,  15,   0, -30}},
    {{-30,   5,  10,  15,  15,  10,   5, -30}},
    {{-40, -20,   0,   5,   5,   0, -20, -40}},
    {{-50, -40, -30, -30, -30, -30, -40, -50}}
}}; 

// bishop
constexpr PST bishop_table = {{
    {{-20, -10, -10, -10, -10, -10, -10, -20}},
    {{-10,   0,   0,   0,   0,   0,   0, -10}},
    {{-10,   0,  10,  10,  10,  10,   0, -10}},
    {{-10,   5,   5,  20,  20,   5,   5, -10}},
    {{-10,   0,  10,  20,  20,  10,   0, -10}},
    {{-10,  10,  10,  10,  10,  10,  10, -10}},
    {{-10,   5,   0,   0,   0,   0,   5, -10}},
    {{-20, -10, -10, -10, -10, -10, -10, -20}}
}};

// rooks
constexpr PST rook_table = {{
    {{  0,   0,   0,   10,   10,   0,   0,   0 }},
    {{  0,   0,   0,   10,   10,   0,   0,   0 }},
    {{  0,   0,   0,   10,   10,   0,   0,   0 }},
    {{  5,   5,   5,   10,   10,   5,   5,   5 }},
    {{  5,   5,   5,   10,   10,   5,   5,   5 }},
    {{  0,   0,   0,   10,   10,   0,   0,   0 }},
    {{  0,   0,   0,   10,   10,   0,   0,   0 }},
    {{  0,   0,   0,   10,   10,   0,   0,   0 }}
}};

// queen
constexpr PST queen_table = {{
    {{-20, -10, -10,  -5,  -5, -10, -10, -20}},
    {{-10,   0,   0,   0,   0,   0,   0, -10}},
    {{-10,   0,   5,   5,   5,   5,   0, -10}},
    {{ -5,   0,   5,   5,   5,   5,   0,  -5}},
    {{  0,   0,   5,   5,   5,   5,   0,  -5}},
    {{-10,   5,   5,   5,   5,   5,   0, -10}},
    {{-10,   0,   5,   0,   0,   0,   0, -10}},
    {{-20, -10, -10,  -5,  -5, -10, -10, -20}}
}};

// king (mid game)
constexpr PST king_mid_table = {{
    {{-30, -40, -40, -50, -50, -40, -40, -30}},
    {{-30, -40, -40, -50, -50, -40, -40, -30}},
    {{-30, -40, -40, -50, -50, -40, -40, -30}},
    {{-30, -40, -40, -50, -50, -40, -40, -30}},
    {{-20, -30, -30, -40, -40, -30, -30, -20}},
    {{-10, -20, -20, -20, -20, -20, -20, -10}},
    {{ 20,  20,   0,   0,   0,   0,  20,  20}},
    {{ 20,  30,  10,   0,   0,  10,  30,  20}}
}};

// king (end game)
constexpr PST king_end_table = {{
    {{-50, -40, -30, -20, -20, -30, -40, -50}},
    {{-30, -20, -101,  0,   0, -10, -20, -30}},
    {{-30, -10,  20,  30,  30,  20, -10, -30}},
    {{-30, -10,  30,  40,  40,  30, -10, -30}},
    {{-30, -10,  30,  40,  40,  30, -10, -30}},
    {{-30, -10,  20,  30,  30,  20, -10, -30}},
    {{-30, -30,   0,   0,   0,   0, -30, -30}},
    {{-50, -30, -30, -30, -30, -30, -30, -50}}
}};

const std::unordered_map<Figures, PST> pst_map = {
    {PAWN, pawn_table},
    {KNIGHT, knight_table},
    {BISHOP, bishop_table},
    {ROOK, rook_table},
    {QUEEN, queen_table}
};

const std::unordered_map<Figures, int> phases_table = {
    {PAWN, 0},
    {KNIGHT, 1},
    {EMPTY, 0},
    {KING,0},
    {ROOK, 2},
    {QUEEN, 4},
    {BISHOP, 1}
};

constexpr int phase_max = 24;

/*{
    white {pawns, doubled pawns, isolated pawns},
    black {pawns, doubled pawns, isolated pawns}
}*/
// возвращает: { white: {pawns, doubled_extra, isolated}, black: {...} }
std::pair<std::array<int, 3>, std::array<int, 3>>
count_pawns(const Position& pos) {
    std::pair<std::array<int, 3>, std::array<int, 3>> res{};
    auto& [white, black] = res;

    std::array<int, 8> wFiles{}; // количество белых пешек на файле
    std::array<int, 8> bFiles{}; // количество чёрных пешек на файле

    // pawns
    for (int file = 0; file < 8; ++file) {
        for (int rank = 0; rank < 8; ++rank) {
            Piece p = pos.getPiece(file, rank);
            if (p.getType() == PAWN) {
                if (p.isWhite()) {
                    white[0]++;
                    wFiles[file]++;
                } else {
                    black[0]++;
                    bFiles[file]++;
                }
            }
        }
    }

    // doubled pawns: считаем лишние пешки на файле (count - 1)
    for (int f = 0; f < 8; ++f) {
        if (wFiles[f] > 1) white[1] += (wFiles[f] - 1);
        if (bFiles[f] > 1) black[1] += (bFiles[f] - 1);
    }

    // isolated pawns (количество пешек без соседей)
    for (int f = 0; f < 8; ++f) {
        if (wFiles[f] > 0) {
            bool left = (f > 0) && wFiles[f - 1] > 0;
            bool right = (f < 7) && wFiles[f + 1] > 0;
            if (!left && !right) white[2] += wFiles[f];
        }
        if (bFiles[f] > 0) {
            bool left = (f > 0) && bFiles[f - 1] > 0;
            bool right = (f < 7) && bFiles[f + 1] > 0;
            if (!left && !right) black[2] += bFiles[f];
        }
    }

    return res;
}

constexpr std::pair<int, int> evaluateKingSafety(const Position& pos, const std::pair<int, int>& wking, const std::pair<int, int>& bking) {
    std::pair<int, int> res = std::make_pair<int, int>(0, 0);
    
    // white
    {
        int penalty = 0;
        // pawn shell
        int dir = 1;
        for (int dx = -1; dx <= 1; dx++) {
            int x = wking.first + dx;
            int y = wking.second - dir;
            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                Piece p = pos.getPiece(x, y);
                if (!(p.getType() == PAWN && p.isWhite())) {
                    penalty += 25;
                }
            }
        }

        // attack weight
        const int attackWeight[6] = {0, 2, 2, 3, 4, 0}; 
        auto mapType = [](Figures f) {
            switch(f) {
                case PAWN:   return 0;
                case KNIGHT: return 1;
                case BISHOP: return 2;
                case ROOK:   return 3;
                case QUEEN:  return 4;
                case KING:   return 5;
                default:     return 0;
            }
        };

        int attackScore = 0;

        const int dirs[8][2] = {
            {1,0},{-1,0},{0,1},{0,-1},
            {1,1},{1,-1},{-1,1},{-1,-1}
        };

        for (auto &d : dirs) {
            int nx = wking.first + d[0];
            int ny = wking.second + d[1];
            if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) continue;
            Piece p = pos.getPiece(nx, ny);
            if (p.getType() != EMPTY && p.isWhite()) {
                attackScore += attackWeight[mapType(p.getType())];
            }
        }

        // attacked squares around the king
        for (auto &d : dirs) {
            int nx = wking.first + d[0];
            int ny = wking.second + d[1];
            if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) continue;
            if (pos.isSquareAttacked({nx, ny})) {
                penalty += 10;
            }
        }

        // transforming attackscore into penalty
        if (attackScore > 0) {
            penalty += attackScore * 15;
        }

        res.first = penalty;
    }
    
    // black
    {
        int penalty = 0;
        // pawn shell
        int dir = 1;
        for (int dx = -1; dx <= 1; dx++) {
            int x = bking.first + dx;
            int y = bking.second - dir;
            if (x >= 0 && x < 8 && y >= 0 && y < 8) {
                Piece p = pos.getPiece(x, y);
                if (!(p.getType() == PAWN && !p.isWhite())) {
                    penalty += 25;
                }
            }
        }

        // attack weight
        const int attackWeight[6] = {0, 2, 2, 3, 4, 0}; 
        auto mapType = [](Figures f) {
            switch(f) {
                case PAWN:   return 0;
                case KNIGHT: return 1;
                case BISHOP: return 2;
                case ROOK:   return 3;
                case QUEEN:  return 4;
                case KING:   return 5;
                default:     return 0;
            }
        };

        int attackScore = 0;

        const int dirs[8][2] = {
            {1,0},{-1,0},{0,1},{0,-1},
            {1,1},{1,-1},{-1,1},{-1,-1}
        };

        for (auto &d : dirs) {
            int nx = bking.first + d[0];
            int ny = bking.second + d[1];
            if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) continue;
            Piece p = pos.getPiece(nx, ny);
            if (p.getType() != EMPTY && !p.isWhite()) {
                attackScore += attackWeight[mapType(p.getType())];
            }
        }

        // attacked squares around the king
        for (auto &d : dirs) {
            int nx = bking.first + d[0];
            int ny = bking.second + d[1];
            if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) continue;
            if (pos.isSquareAttacked({nx, ny})) {
                penalty += 10;
            }
        }

        // transforming attackscore into penalty
        if (attackScore > 0) {
            penalty += attackScore * 15;
        }

        res.second = penalty;
    }
    
    return res;
}

int evaluate(const Position& pos) {
    // material values in centipawns
    const std::unordered_map<Figures, int> pieceValue = {
        {PAWN, 100},
        {KNIGHT, 320},
        {BISHOP, 330},
        {ROOK, 500},
        {QUEEN, 900},
        {KING, 20000}
    };

    std::pair<int,int> wking = {-1,-1}, bking = {-1,-1};
    int curphase = 0;

    int materialW = 0, materialB = 0;
    int pstW = 0, pstB = 0;
    int pawnPenW = 0, pawnPenB = 0;
    int kingEvalW = 0, kingEvalB = 0;
    int kingSafetyW = 0, kingSafetyB = 0;

    // iterate board: file (x) 0..7, rank (y) 0..7
    for (int file = 0; file < 8; ++file) {
        for (int rank = 0; rank < 8; ++rank) {
            Piece piece = pos.getPiece(file, rank);
            Figures t = piece.getType();
            if (t == EMPTY) continue;

            if (t == KING) {
                if (piece.isWhite()) wking = {file, rank};
                else bking = {file, rank};
                continue;
            }

            int base = 0;
            auto it = pieceValue.find(t);
            if (it != pieceValue.end()) base = it->second;

            // PST: теперь таблицы трактуются как PST[rank][file]
            int pstv = 0;
            auto itpst = pst_map.find(t);
            if (itpst != pst_map.end()) {
                if (piece.isWhite()) {
                    pstv = itpst->second[rank][file];
                } else {
                    // для чёрных зеркалим по рангу
                    pstv = itpst->second[7 - rank][file];
                }
            }

            if (piece.isWhite()) {
                materialW += base;
                pstW += pstv;
            } else {
                materialB += base;
                pstB += pstv;
            }

            auto itph = phases_table.find(t);
            if (itph != phases_table.end()) curphase += itph->second;
        }
    }

    // blended king eval
    int phase = std::min(curphase, phase_max);
    int mid_weight = phase;
    int end_weight = phase_max - phase;

    if (wking.first >= 0) {
        int mid = KING + king_mid_table[wking.second][wking.first];   // note: indexing rank,file if tables defined that way
        int end = KING + king_end_table[wking.second][wking.first];
        kingEvalW = (mid * mid_weight + end * end_weight) / phase_max;
    }
    if (bking.first >= 0) {
        int mid = KING + king_mid_table[7 - bking.second][bking.first]; // mirror for black
        int end = KING + king_end_table[7 - bking.second][bking.first];
        kingEvalB = (mid * mid_weight + end * end_weight) / phase_max;
    }

    // pawns and penalties
    auto pawns = count_pawns(pos);
    auto whiteP = pawns.first;
    auto blackP = pawns.second;

    pawnPenW = whiteP[1] * 12 + whiteP[2] * 15;
    pawnPenB = blackP[1] * 12 + blackP[2] * 15;

    auto king_pen = evaluateKingSafety(pos, wking, bking);
    kingSafetyW = king_pen.first;
    kingSafetyB = king_pen.second;

    int wm = materialW + pstW + kingEvalW - pawnPenW - kingSafetyW;
    int bm = materialB + pstB + kingEvalB - pawnPenB - kingSafetyB;

    int score = wm - bm;

    constexpr int INF = 1000000000;
    if (score > INF-1) score = INF-1;
    if (score < -INF+1) score = -INF+1;
    return score;
}