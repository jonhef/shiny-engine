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

    whiteMove = true;
    whiteCastled = false;
    blackCastled = false;

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

    this->longWhiteCastling = pos.longWhiteCastling;
    this->shortWhiteCastling = pos.shortWhiteCastling;
    this->longBlackCastling = pos.longBlackCastling;
    this->shortBlackCastling = pos.shortBlackCastling;

    this->whiteCastled = pos.whiteCastled;
    this->blackCastled = pos.blackCastled;
    this->whiteMove = pos.whiteMove;

    this->enPassant = pos.enPassant;
    this->enPassantX = pos.enPassantX;
    this->enPassantY = pos.enPassantY;
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

    this->longWhiteCastling = pos.longWhiteCastling;
    this->shortWhiteCastling = pos.shortWhiteCastling;
    this->longBlackCastling = pos.longBlackCastling;
    this->shortBlackCastling = pos.shortBlackCastling;

    this->whiteCastled = pos.whiteCastled;
    this->blackCastled = pos.blackCastled;
    this->whiteMove = pos.whiteMove;

    this->enPassant = pos.enPassant;
    this->enPassantX = pos.enPassantX;
    this->enPassantY = pos.enPassantY;
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

const Board& Position::operator[](Figures figure) const {
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

const Board& Position::operator[](int figure) const {
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
bool Position::hasEnPassant() const { return this->enPassant; }
int& Position::getEnPassantX() { return this->enPassantX; }
int& Position::getEnPassantY() { return this->enPassantY; }
std::pair<int, int> Position::getEnPassantSquare() const { return {this->enPassantX, this->enPassantY}; }

void Position::setEnPassantSquare(std::pair<int, int> square) {
    enPassantX = square.first;
    enPassantY = square.second;
}

bool Position::isTerminal() const {
    return getLegalMoves(*this).empty();
}

Figures Position::getPieceAt(int x, int y) const {
    if (white_pawns[x][y]) return WHITE_PAWN;
    if (black_pawns[x][y]) return BLACK_PAWN;
    if (white_bishops[x][y]) return WHITE_BISHOP;
    if (black_bishops[x][y]) return BLACK_BISHOP;
    if (white_knights[x][y]) return WHITE_KNIGHT;
    if (black_knights[x][y]) return BLACK_KNIGHT;
    if (white_kings[x][y]) return WHITE_KING;
    if (black_kings[x][y]) return BLACK_KING;
    if (white_rooks[x][y]) return WHITE_ROOK;
    if (black_rooks[x][y]) return BLACK_ROOK;
    if (white_queens[x][y]) return WHITE_QUEEN;
    if (black_queens[x][y]) return BLACK_QUEEN;
    return EMPTY;
}