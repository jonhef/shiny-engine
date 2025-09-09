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
    {{-10,   0,   5,  10,  10,   5,   0, -10}},
    {{-10,   5,   5,  10,  10,   5,   5, -10}},
    {{-10,   0,  10,  10,  10,  10,   0, -10}},
    {{-10,  10,  10,  10,  10,  10,  10, -10}},
    {{-10,   5,   0,   0,   0,   0,   5, -10}},
    {{-20, -10, -10, -10, -10, -10, -10, -20}}
}};

// rooks
constexpr PST rook_table = {{
    {{  0,   0,   0,   0,   0,   0,   0,   0 }},
    {{  5,  10,  10,  10,  10,  10,  10,   5 }},
    {{ -5,   0,   0,   0,   0,   0,   0,  -5 }},
    {{ -5,   0,   0,   0,   0,   0,   0,  -5 }},
    {{ -5,   0,   0,   0,   0,   0,   0,  -5 }},
    {{ -5,   0,   0,   0,   0,   0,   0,  -5 }},
    {{ -5,   0,   0,   0,   0,   0,   0,  -5 }},
    {{  0,   0,   0,   5,   5,   0,   0,   0 }}
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
    {QUEEN, 4}
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
        bm += mid_eval;
    }

    // black king
    {
        int mid_eval = (
            (KING + mirror(king_mid_table)[wking.first][wking.second]) * mid_weight +
            (KING + mirror(king_end_table)[wking.first][wking.second]) * end_weight
        ) / phase_max;
        bm += mid_eval;
    }

    return res;
}