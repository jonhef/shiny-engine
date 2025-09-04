#include "board.h"

Row::Row() {
    for (int i = 0; i < 8; ++i) {
        this->row[i] = false;
    }
}

Row::Row(int row[8]) {
    for (int i = 0; i < 8; ++i) {
        this->row[i] = (bool)row[i];
    }
}

Row::~Row() {   }

bool& Row::operator[](int index) {
    return this->row[index];
}

bool Row::operator[](int index) const {
    return this->row[index];
}

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

const Row& Board::operator[](int index) const {
    return board[index];
}