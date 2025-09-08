
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
    this->x = x % 8;
    this->y = y % 8;
}

Piece::~Piece() {
}

int Piece::isWhite() const {
    return color;
}

std::pair<int, int> Piece::getPos() const {
    return std::make_pair(x % 8, y % 8);
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
    this->x = x % 8;
    this->y = y % 8;
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

Position::Position(
    std::array<std::array<Piece, 8>, 8>      board,
    bool                               isWhiteMove,
    bool                  shortWhiteCastle = false,
    bool                   longWhiteCastle = false,
    bool                  shortBlackCastle = false,
    bool                   longBlackCastle = false,
    std::pair<int, int> squareEnPassant = {-1, -1}
) {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            this->board[i][j] = board[i][j];
        }
    }

    this->castleRights = 0;
    castleRights |= (int)shortWhiteCastle;
    castleRights |= (int)longWhiteCastle << 1;
    castleRights |= (int)shortBlackCastle << 2;
    castleRights |= (int)longBlackCastle << 3;

    this->squareEnPassant.first = squareEnPassant.first;
    this->squareEnPassant.second = squareEnPassant.second;
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

bool Position::isSquareAttacked(const std::pair<int, int>& pos) const {
    int x = pos.first;
    int y = pos.second;
    bool attackerColor = !isWhiteMove;

    // Pawn
    {
        int dir = attackerColor ? 1 : -1; 
        int px[2] = {x - 1, x + 1};
        int py = y - dir;
        for (int i = 0; i < 2; i++) {
            int nx = px[i], ny = py;
            if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                Piece p = getPiece(nx, ny);
                if (p.getType() == PAWN && p.isWhite() == attackerColor)
                    return true;
            }
        }
    }

    // Knight
    {
        const int knightMoves[8][2] = {
            {1,2},{2,1},{-1,2},{-2,1},
            {1,-2},{2,-1},{-1,-2},{-2,-1}
        };
        for (auto &m : knightMoves) {
            int nx = x + m[0], ny = y + m[1];
            if (nx>=0 && nx<8 && ny>=0 && ny<8) {
                Piece p = getPiece(nx, ny);
                if (p.getType() == KNIGHT && p.isWhite() == attackerColor)
                    return true;
            }
        }
    }

    // King
    {
        const int kingMoves[8][2] = {
            {1,0},{-1,0},{0,1},{0,-1},
            {1,1},{1,-1},{-1,1},{-1,-1}
        };
        for (auto &m : kingMoves) {
            int nx = x + m[0], ny = y + m[1];
            if (nx>=0 && nx<8 && ny>=0 && ny<8) {
                Piece p = getPiece(nx, ny);
                if (p.getType() == KING && p.isWhite() == attackerColor)
                    return true;
            }
        }
    }

    // Diagonals(queen and bishop)
    {
        const int diagDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
        for (auto &d : diagDirs) {
            int nx = x + d[0], ny = y + d[1];
            while (nx>=0 && nx<8 && ny>=0 && ny<8) {
                Piece p = getPiece(nx, ny);
                if (p.getType() != EMPTY) {
                    if ((p.getType() == BISHOP || p.getType() == QUEEN) && p.isWhite() == attackerColor)
                        return true;
                    break;
                }
                nx += d[0];
                ny += d[1];
            }
        }
    }

    // Straight(rook and queen)
    {
        const int lineDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        for (auto &d : lineDirs) {
            int nx = x + d[0], ny = y + d[1];
            while (nx>=0 && nx<8 && ny>=0 && ny<8) {
                Piece p = getPiece(nx, ny);
                if (p.getType() != EMPTY) {
                    if ((p.getType() == ROOK || p.getType() == QUEEN) && p.isWhite() == attackerColor)
                        return true;
                    break;
                }
                nx += d[0];
                ny += d[1];
            }
        }
    }

    return false;
}

bool Position::isCheck() const {
    std::pair<int, int> kingPos(-1, -1);


    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (board[i][j].getType() == KING && this->isWhiteMove == board[i][j].isWhite()) {
                kingPos.first = i;
                kingPos.second = j;
                break;
            }
        }
        if (kingPos.first != -1) {
            break;
        }
    }

    if (this->isSquareAttacked(kingPos)) {
        return true;
    }
    return false;
}