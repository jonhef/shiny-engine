#include "position.h"
#include "figures.h"
#include "chess_logic.h"

Position::Position() {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            white_pawns[i][j] = 0;
            white_knights[i][j] = 0;
            white_bishops[i][j] = 0;
            white_rooks[i][j] = 0;
            white_queens[i][j] = 0;
            white_kings[i][j] = 0;

            black_pawns[i][j] = 0;
            black_knights[i][j] = 0;
            black_bishops[i][j] = 0;
            black_rooks[i][j] = 0;
            black_queens[i][j] = 0;
            black_kings[i][j] = 0;
        }
    }
    longWhiteCastling = shortWhiteCastling = false;
    longBlackCastling = shortBlackCastling = false;

    enPassant = false;
    enPassantX = -1;  // NEW
    enPassantY = -1;
}

Position::Position(Position& pos) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            this->white_pawns[i][j] = pos.white_pawns[i][j];
            this->white_knights[i][j] = pos.white_knights[i][j];
            this->white_bishops[i][j] = pos.white_bishops[i][j];
            this->white_rooks[i][j] = pos.white_rooks[i][j];
            this->white_queens[i][j] = pos.white_queens[i][j];
            this->white_kings[i][j] = pos.white_kings[i][j];

            this->black_pawns[i][j] = pos.black_pawns[i][j];
            this->black_knights[i][j] = pos.black_knights[i][j];
            this->black_bishops[i][j] = pos.black_bishops[i][j];
            this->black_rooks[i][j] = pos.black_rooks[i][j];
            this->black_queens[i][j] = pos.black_queens[i][j];
            this->black_kings[i][j] = pos.black_kings[i][j];
        }
    }
}

Position::Position(const Position& pos) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            this->white_pawns[i][j] = pos.white_pawns[i][j];
            this->white_knights[i][j] = pos.white_knights[i][j];
            this->white_bishops[i][j] = pos.white_bishops[i][j];
            this->white_rooks[i][j] = pos.white_rooks[i][j];
            this->white_queens[i][j] = pos.white_queens[i][j];
            this->white_kings[i][j] = pos.white_kings[i][j];

            this->black_pawns[i][j] = pos.black_pawns[i][j];
            this->black_knights[i][j] = pos.black_knights[i][j];
            this->black_bishops[i][j] = pos.black_bishops[i][j];
            this->black_rooks[i][j] = pos.black_rooks[i][j];
            this->black_queens[i][j] = pos.black_queens[i][j];
            this->black_kings[i][j] = pos.black_kings[i][j];
        }
    }
}

Position::~Position() {
}

Board& Position::operator[](Figures figure) {
    switch (figure) {
        case WHITE_PAWN:   return white_pawns;
        case WHITE_KNIGHT: return white_knights;
        case WHITE_BISHOP: return white_bishops;
        case WHITE_ROOK:   return white_rooks;
        case WHITE_QUEEN:  return white_queens;
        case WHITE_KING:   return white_kings;
        case BLACK_PAWN:   return black_pawns;
        case BLACK_KNIGHT: return black_knights;
        case BLACK_BISHOP: return black_bishops;
        case BLACK_ROOK:   return black_rooks;
        case BLACK_QUEEN:  return black_queens;
        case BLACK_KING:   return black_kings;
        default:           return white_pawns;
    }
}

Board Position::operator[](Figures figure) const {
    switch (figure) {
        case WHITE_PAWN:   return white_pawns;
        case WHITE_KNIGHT: return white_knights;
        case WHITE_BISHOP: return white_bishops;
        case WHITE_ROOK:   return white_rooks;
        case WHITE_QUEEN:  return white_queens;
        case WHITE_KING:   return white_kings;
        case BLACK_PAWN:   return black_pawns;
        case BLACK_KNIGHT: return black_knights;
        case BLACK_BISHOP: return black_bishops;
        case BLACK_ROOK:   return black_rooks;
        case BLACK_QUEEN:  return black_queens;
        case BLACK_KING:   return black_kings;
        default:           return white_pawns;
    }
}

Board& Position::operator[](int figure) {
    switch (figure) {
        case WHITE_PAWN:   return white_pawns;
        case WHITE_KNIGHT: return white_knights;
        case WHITE_BISHOP: return white_bishops;
        case WHITE_ROOK:   return white_rooks;
        case WHITE_QUEEN:  return white_queens;
        case WHITE_KING:   return white_kings;
        case BLACK_PAWN:   return black_pawns;
        case BLACK_KNIGHT: return black_knights;
        case BLACK_BISHOP: return black_bishops;
        case BLACK_ROOK:   return black_rooks;
        case BLACK_QUEEN:  return black_queens;
        case BLACK_KING:   return black_kings;
        default:           return white_pawns;
    }
}

Board Position::operator[](int figure) const {
    switch (figure) {
        case WHITE_PAWN:   return white_pawns;
        case WHITE_KNIGHT: return white_knights;
        case WHITE_BISHOP: return white_bishops;
        case WHITE_ROOK:   return white_rooks;
        case WHITE_QUEEN:  return white_queens;
        case WHITE_KING:   return white_kings;
        case BLACK_PAWN:   return black_pawns;
        case BLACK_KNIGHT: return black_knights;
        case BLACK_BISHOP: return black_bishops;
        case BLACK_ROOK:   return black_rooks;
        case BLACK_QUEEN:  return black_queens;
        case BLACK_KING:   return black_kings;
        default:           return white_pawns;
    }
}

bool& Position::getLongWhiteCastling() {
    return this->longWhiteCastling;
}

bool Position::getLongWhiteCastling() const {
    return this->longWhiteCastling;
}

bool& Position::getShortWhiteCastling() {
    return this->shortWhiteCastling;
}

bool Position::getShortWhiteCastling() const {
    return this->shortWhiteCastling;
}

bool& Position::getLongBlackCastling() {
    return this->longBlackCastling;
}

bool Position::getLongBlackCastling() const {
    return this->longBlackCastling;
}

bool& Position::getShortBlackCastling() {
    return this->shortBlackCastling;
}

bool Position::getShortBlackCastling() const {
    return this->shortBlackCastling;
}

bool& Position::isWhiteCastled() {
    return this->whiteCastled;
}

bool Position::isWhiteCastled() const {
    return this->whiteCastled;
}

bool& Position::isBlackCastled() {
    return this->blackCastled;
}

bool Position::isBlackCastled() const {
    return this->blackCastled;
}

bool& Position::isWhiteMove() {
    return this->whiteMove;
}

bool Position::isWhiteMove() const {
    return this->whiteMove;
}

// NEW
bool& Position::hasEnPassant() { return this->enPassant; }
int& Position::getEnPassantX() { return this->enPassantX; }
int& Position::getEnPassantY() { return this->enPassantY; }
std::pair<int, int> Position::getEnPassantSquare() const { return {this->enPassantX, this->enPassantY}; }

void Position::setEnPassantSquare(std::pair<int, int> square) {
    enPassantX = square.first;
    enPassantY = square.second;
}

bool Position::isTerminal() const {
    return isCheckmate(*this, this->whiteMove);
}