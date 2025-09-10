#include "evaluation.h"
#include <array>
#include <unordered_map>
#include <algorithm>

/* Helpers */
using PST = std::array<std::array<int, 8>, 8>;

// pawn
constexpr PST pawn_table = {{ 
    {{  0,   0,   0,   0,   0,   0,   0,   0 }},
    {{ 50,  50,  50,  50,  50,  50,  50,  50 }},
    {{ 10,  10,  20,  30,  30,  20,  10,  10 }},
    {{  5,   5,  10,  25,  25,  10,   5,   5 }},
    {{  0,   0,   0,  20,  20,   0,   0,   0 }},
    {{  5,  -5, -10,   0,   0, -10,  -5,   5 }},
    {{  5,  10,  10, -20, -20,  10,  10,   5 }},
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

constexpr PST mirror(const PST& table) {
    PST mirrored = {};
    for (int file = 0; file < 8; file++) {
        for (int rank = 0; rank < 8; rank++) {
            mirrored[file][rank] = table[file][7 - rank];
        }
    }
    return mirrored;
}

/*{
    white {pawns, doubled pawns, isolated pawns},
    black {pawns, doubled pawns, isolated pawns}
}*/
constexpr std::pair<std::array<int, 3>, std::array<int, 3>>
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

    // doubled pawns
    for (int f = 0; f < 8; ++f) {
        white[1] += (wFiles[f] > 1) ? wFiles[f] : 0;
        black[1] += (bFiles[f] > 1) ? bFiles[f] : 0;
    }

    // isolated pawns
    for (int f = 0; f < 8; ++f) {
        if (wFiles[f] > 0) {
            bool left = (f > 0) && wFiles[f - 1] > 0;
            bool right = (f < 7) && wFiles[f + 1] > 0;
            white[2] += (!left && !right) ? wFiles[f] : 0;
        }
        if (bFiles[f] > 0) {
            bool left = (f > 0) && bFiles[f - 1] > 0;
            bool right = (f < 7) && bFiles[f + 1] > 0;
            black[2] += (!left && !right) ? bFiles[f] : 0;
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
    // material evaluation
    int res = 0;
    
    std::pair<int, int> wking, bking;
    int curphase = 0;

    int bm = 0, wm = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            Piece piece = pos.getPiece(i, j);
            int m = 0;
            if (piece.getType() == KING) {
                if (piece.isWhite()) {
                    wking = std::make_pair(i, j);
                } else {
                    bking = std::make_pair(i, j);
                }
                continue;
            }
            m += piece.getType();

            if (piece.getType() == EMPTY) {
                continue;
            }
            
            if (piece.isWhite()) {
                m += pst_map.at(piece.getType())[i][j];
                wm += m;
            } else {
                m += mirror(pst_map.at(piece.getType()))[i][j];
                bm += m;
            }

            curphase += phases_table.at(piece.getType());
        }
    }

    int phase = std::min(curphase, phase_max);
    int mid_weight = phase;
    int end_weight = phase_max - phase;

    // white king
    {
        int mid_eval = (
            (KING + king_mid_table[wking.first][wking.second]) * mid_weight +
            (KING + king_end_table[wking.first][wking.second]) * end_weight
        ) / phase_max;
        wm += mid_eval;
    }

    // black king
    {
        int mid_eval = (
            (KING + mirror(king_mid_table)[wking.first][wking.second]) * mid_weight +
            (KING + mirror(king_end_table)[bking.first][bking.second]) * end_weight
        ) / phase_max;
        bm += mid_eval;
    }

    auto pawns = count_pawns(pos);

    wm -= pawns.first[1] * 12;
    wm -= pawns.first[2] * 15;
    
    bm -= pawns.first[1] * 12;
    bm -= pawns.first[2] * 12;

    auto king_penalty = evaluateKingSafety(pos, wking, bking);

    wm -= king_penalty.first;
    bm -= king_penalty.second;

    return wm - bm;
}