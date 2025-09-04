#include "ratio.h"
#include <math.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

Ratio::Ratio() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            coeffecients[i][j] = 1;
        }
    }
}

Ratio::Ratio(double coeffecients[8][8], Figures piece) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            this->coeffecients[i][j] = coeffecients[i][j];
        }
    }
    this->piece = piece;
}

Ratio::Ratio(Figures piece) {
    switch (piece) {
        case WHITE_PAWN:
            for (int i = 0; i < 8; ++i) { // от 0 до 7
                for (int j = 0; j < 8; ++j) {
                    coeffecients[i][j] = pow(1.25, i); // белые пешки растут по рядам
                }
            }
            break;

        case BLACK_PAWN:
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    coeffecients[i][j] = pow(1.25, 7 - i); // зеркально для черных
                }
            }
            break;

        case WHITE_KNIGHT:
        case BLACK_KNIGHT:
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    int dist_i = std::abs(3 - i); // расстояние от центра по i
                    int dist_j = std::abs(3 - j); // расстояние от центра по j
                    int min_dist = std::min(dist_i, dist_j);
                    coeffecients[i][j] = pow(1.25, 3 - min_dist); // центр = макс, края = меньше
                }
            }
            break;
        case WHITE_BISHOP:
        case BLACK_BISHOP:
        case WHITE_QUEEN:
        case BLACK_QUEEN: {
            double coefs[8][8] = {
                {1.25, 1.10, 1.05, 0.95, 0.95, 1.05, 1.15, 1.25},
                {1.15, 1.25, 1.12, 1.05, 1.05, 1.15, 1.25, 1.15},
                {1.05, 1.15, 1.25, 1.15, 1.15, 1.25, 1.15, 1.05},
                {0.95, 1.05, 1.15, 1.25, 1.25, 1.15, 1.05, 0.95},
                {0.95, 1.05, 1.15, 1.25, 1.25, 1.15, 1.05, 0.95},
                {1.05, 1.15, 1.25, 1.15, 1.15, 1.25, 1.12, 1.05},
                {1.10, 1.25, 1.25, 1.15, 1.15, 1.25, 1.25, 1.10},
                {1.25, 1.15, 1.05, 0.95, 0.95, 1.05, 1.15, 1.25}
            };
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    coeffecients[i][j] = coefs[i][j];
                }
            }
            break;
        }

        case WHITE_ROOK:
        case BLACK_ROOK:
        case EMPTY:
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    coeffecients[i][j] = 1.0;
                }
            }
            break;

        case WHITE_KING:
        case BLACK_KING: {
            double min_coef = 0.7;
            double max_coef = 1.1;
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    // расстояние от центра доски
                    double dist = sqrt(pow(i - 3.5, 2) + pow(j - 3.5, 2));
                    double max_dist = sqrt(2 * pow(3.5, 2));
                    double coef = max_coef - (max_coef - min_coef) * (dist / max_dist);
                    // Белый король положительный, черный отрицательный
                    coeffecients[i][j] = coef;
                }
            }
            break;
        }
    }

    this->piece = piece;
}


Ratio::~Ratio() {
}

double Ratio::operator*(const Position& pos) {
    Board board (pos[this->piece]);
    double result = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j]) {
                result += coeffecients[i][j] * piece;
            }
        }
    }
    return result;
}