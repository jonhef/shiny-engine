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
            for (int i = 1; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    coeffecients[i][j] = pow(1.25, (double)i - 1);
                }
            }
            break;
        case BLACK_KNIGHT:
        case WHITE_KNIGHT:
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    coeffecients[i][j] = (double)1.35 - (double)0.05 * pow(3.5 - (double)MIN(i, j), 2);
                }
            }
            break;
        case BLACK_QUEEN:
        case WHITE_QUEEN:
        case BLACK_BISHOP:
        case WHITE_BISHOP: {
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
            Ratio(coefs, piece);
            break;
        }
        case EMPTY:
        case BLACK_ROOK:
        case WHITE_ROOK:
            Ratio();
            break;
        case BLACK_KING:
        case WHITE_KING:
            for (int i = 0; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    coeffecients[i][j] = 1 - ((double).35 + (double)0.05 * pow(3.5 - (double)MIN(i, j), 2));
                }
            }
            break;
        case BLACK_PAWN:
            for (int i = 1; i < 8; ++i) {
                for (int j = 0; j < 8; ++j) {
                    coeffecients[i][j] = pow(1.25, 7 - i);
                }
            }
    }
    this->piece = piece;
}

Ratio::~Ratio() {
}

double Ratio::operator*(const Position pos) {
    Board board (pos[this->piece]);
    double result = 0;
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            result += coeffecients[i][j] * (board[i][j] ? piece : 0);
        }
    }
    return result;
}