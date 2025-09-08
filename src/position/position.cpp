
#include "position.h"

Piece::Piece() {
    type = PAWN;
    color = true;
    x = 0;
    y = 0;
}

Piece::Piece(Figures figure, bool color) {
    type = figure;
    this->color = color;
    x = 0;
    y = 0;
}

Piece::Piece(Figures figure, bool color, int x, int y) {
    type = figure;
    this->color = color;
    this->x = x;
    this->y = y;
}

Piece::~Piece() {
}

int Piece::isWhite() const {
    return color;
}

std::pair<int, int> Piece::getPos() const {
    return std::make_pair(x, y);
}

Figures Piece::getType() const {
    return type;
}


void Piece::setColor(bool color) {
    this->color = color;
}

void Piece::setPos(const std::pair<int, int>& pos) {
    x = pos.first;
    y = pos.second;
}

void Piece::setPos(int x, int y) {
    this->x = x;
    this->y = y;
}

void Piece::setType(Figures figure) {
    type = figure;
}

Position::Position() {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = Piece();
        }
    }
}

Position::~Position() {
}

/* x = 0, y = 0 is a1 */
Piece Position::getPiece(int x, int y) const {
    return board[x][y];
}

/* it changes original piece's position 
   x = 0, y = 0 is a1 */
void Position::setPiece(int x, int y, Piece& piece) {
    board[x][y] = piece;
}

/* x = 0, y = 0 is a1 */
void Position::setPiece(int x, int y, Figures figure, bool color) {
    board[x][y] = Piece(figure, color);
}