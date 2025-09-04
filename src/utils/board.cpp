#include "board.h"

Board::Board() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            board[i][j] = 0;
        }
    }
}

Board::Board(Board& board) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            this->board[i][j] = board[i][j];
        }
    }
}

Board::Board(const Board& board) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            this->board[i][j] = board[i][j];
        }
    }
}

Board::~Board() {
}

Row& Board::operator[](int index) {
    return board[index];
}

Row Board::operator[](int index) const {
    return board[index];
}