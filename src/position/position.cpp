#include "position.h"
#include <utility>

Piece::Piece() {
    type = EMPTY;
    color = false;
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
            board[i][j] = Piece(EMPTY, false);
        }
    }

    this->squareEnPassant = std::make_pair<int, int>(-1, -1);
    this->castleRights = 0;
    this->isWhiteMove = true; 
}

Position::Position(
    std::array<std::array<Piece, 8>, 8>      board,
    bool                               isWhiteMove,
    bool                          shortWhiteCastle,
    bool                           longWhiteCastle,
    bool                          shortBlackCastle,
    bool                           longBlackCastle,
    std::pair<int, int>            squareEnPassant
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

    this->squareEnPassant = squareEnPassant;

    this->isWhiteMove = isWhiteMove;
}

Position::~Position() {
}

/* it changes original piece's position 
   x = 0, y = 0 is a1 */
void Position::setPiece(int x, int y, Piece piece) {
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

    if (kingPos.first == -1) {
        return false; 
    }

    if (this->isSquareAttacked(kingPos)) {
        return true;
    }
    return false;
}

void Position::setIsWhiteMove(bool side) {
    this->isWhiteMove = side;
}

void Position::setCastleRights(short castleRights) {
    this->castleRights = castleRights;
}

short Position::getCastleRights() const {
    return this->castleRights;
}

bool Position::isWhiteToMove() const {
    return this->isWhiteMove;
}

void Position::setEnPassant(int x, int y) {
    if (x < 0 || x >= 8 || y < 0 || y >= 8) {
        this->squareEnPassant = {-1, -1};
        return;
    }
    this->squareEnPassant = {x, y};
}

void Position::setEnPassant(std::pair<int, int> enPassant) {
    this->setEnPassant(enPassant.first, enPassant.second);
}

std::pair<int, int> Position::getEnPassant() const {
    return this->squareEnPassant;
}

// --- локальные помощники ---
static inline bool inBoard(int x, int y) { return x>=0 && x<8 && y>=0 && y<8; }

static inline void add_promotion_or_push(std::vector<Move>& out, int fx,int fy,int tx,int ty, bool needPromo) {
    if (!needPromo) {
        out.push_back({fx,fy,tx,ty,EMPTY,false,false,false});
    } else {
        for (Figures pr : {QUEEN, ROOK, BISHOP, KNIGHT})
            out.push_back({fx,fy,tx,ty,pr,false,false,false});
    }
}

// применить ход в копии позиции (минимально достаточно для проверки шаха)
static void applyMove(Position& pos, const Move& m) {
    bool moverIsWhite = pos.isWhiteToMove();
    Piece moving = pos.getPiece(m.fromX, m.fromY);

    // очистить начальную клетку
    pos.setPiece(m.fromX, m.fromY, Piece(EMPTY, WHITE));

    // --- снять фигуру, если нужно (обычное взятие) ---
    if (!(m.isEnPassant || m.isCastleShort || m.isCastleLong)) {
        Piece target = pos.getPiece(m.toX, m.toY);
        if (target.getType() != EMPTY) {
            // если взяли ладью на угловом поле, снять права на рокировку
            if (target.getType() == ROOK) {
                if (target.isWhite()) {
                    if (m.toX == 0 && m.toY == 0) pos.setCastleRights(pos.getCastleRights() & ~0b0010); // a1
                    if (m.toX == 7 && m.toY == 0) pos.setCastleRights(pos.getCastleRights() & ~0b0001); // h1
                } else {
                    if (m.toX == 0 && m.toY == 7) pos.setCastleRights(pos.getCastleRights() & ~0b1000); // a8
                    if (m.toX == 7 && m.toY == 7) pos.setCastleRights(pos.getCastleRights() & ~0b0100); // h8
                }
            }
        }
    }

    // --- специальные случаи ---
    if (m.isEnPassant) {
        int dir = moverIsWhite ? 1 : -1;
        pos.setPiece(m.toX, m.toY - dir, Piece(EMPTY, WHITE));
    } else if (m.isCastleShort) {
        int y = moverIsWhite ? 0 : 7;
        Piece rook = pos.getPiece(7, y);
        pos.setPiece(7, y, Piece(EMPTY, WHITE));
        pos.setPiece(5, y, rook);
    } else if (m.isCastleLong) {
        int y = moverIsWhite ? 0 : 7;
        Piece rook = pos.getPiece(0, y);
        pos.setPiece(0, y, Piece(EMPTY, WHITE));
        pos.setPiece(3, y, rook);
    }

    // --- поставить фигуру на целевую ---
    if (m.promotion != EMPTY) moving.setType(m.promotion);
    moving.setPos(m.toX, m.toY);
    pos.setPiece(m.toX, m.toY, moving);

    // --- обновить права на рокировку для своей стороны ---
    if (moving.getType() == KING) {
        if (moverIsWhite) {
            pos.setCastleRights(pos.getCastleRights() & ~0b0011); // убираем оба белых флага
        } else {
            pos.setCastleRights(pos.getCastleRights() & ~0b1100); // убираем оба чёрных флага
        }
    }
    if (moving.getType() == ROOK) {
        if (moverIsWhite) {
            if (m.fromX == 0 && m.fromY == 0) pos.setCastleRights(pos.getCastleRights() & ~0b0010); // a1
            if (m.fromX == 7 && m.fromY == 0) pos.setCastleRights(pos.getCastleRights() & ~0b0001); // h1
        } else {
            if (m.fromX == 0 && m.fromY == 7) pos.setCastleRights(pos.getCastleRights() & ~0b1000); // a8
            if (m.fromX == 7 && m.fromY == 7) pos.setCastleRights(pos.getCastleRights() & ~0b0100); // h8
        }
    }

    // --- обновить en passant ---
    pos.setEnPassant(-1,-1);
    if (moving.getType() == PAWN && std::abs(m.toY - m.fromY) == 2) {
        int midRank = (m.toY + m.fromY) / 2;
        pos.setEnPassant(m.fromX, midRank);
    }

    // --- переключить ход ---
    pos.setIsWhiteMove(!moverIsWhite);
}

void Position::applyMove(const Move& move) {
    ::applyMove(*this, move);
}

// проверка: клетка (x,y) не атакована соперником цвета `attackerIsWhite`?
static bool squareSafeFor(const Position& base, int x, int y, bool defenderIsWhite) {
    Position tmp = base;
    // isSquareAttacked берёт цвет атакующего как !isWhiteMove,
    // значит isWhiteMove должен быть цветом защищающегося
    tmp.setIsWhiteMove(defenderIsWhite);
    return !tmp.isSquareAttacked({x,y});
}

// корректная рокировка с полными проверками
static void genCastling(const Position& pos, bool side, int kx, int ky, std::vector<Move>& out) {
    // король должен стоять на исходной клетке, но опираться будем на castleRights + пустые/неатакованные клетки
    short cr = pos.getCastleRights();

    // нельзя рокироваться если король сейчас под шахом
    if (!squareSafeFor(pos, kx, ky, side)) return;

    // short (king-side)
    if ((side && (cr & 0b0001)) || (!side && (cr & 0b0100))) {
        int y = side ? 0 : 7;
        // клетки между: f,g (5, y) и (6, y) пустые
        if (pos.getPiece(5, y).getType() == EMPTY &&
            pos.getPiece(6, y).getType() == EMPTY) {
            // и не атакованы
            if (squareSafeFor(pos, 5, y, side) && squareSafeFor(pos, 6, y, side)) {
                out.push_back({kx, ky, 6, y, EMPTY, false, true, false});
            }
        }
    }
    // long (queen-side)
    if ((side && (cr & 0b0010)) || (!side && (cr & 0b1000))) {
        int y = side ? 0 : 7;
        // клетки между: b,c,d — (1,y) может быть занята ладьёй; обязательно пустые c,d (2,3)
        if (pos.getPiece(1, y).getType() == EMPTY ||
            pos.getPiece(1, y).getType() == ROOK) { /* допустим, но не обязательно пустая */ }
        if (pos.getPiece(2, y).getType() == EMPTY &&
            pos.getPiece(3, y).getType() == EMPTY) {
            if (squareSafeFor(pos, 3, y, side) && squareSafeFor(pos, 2, y, side)) {
                out.push_back({kx, ky, 2, y, EMPTY, false, false, true});
            }
        }
    }
}

std::vector<Move> Position::getLegalMoves() const {
    std::vector<Move> pseudo;
    const bool white = isWhiteToMove();

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            Piece p = getPiece(x, y);
            if (p.getType() == EMPTY) continue;
            if (p.isWhite() != white) continue;

            switch (p.getType()) {
                case PAWN: {
                    int dir = white ? 1 : -1;
                    int startRank = white ? 1 : 6;
                    int promoteRank = white ? 7 : 0;

                    // вперед 1 (по Y)
                    int fy = y + dir;
                    if (inBoard(x, fy) && getPiece(x, fy).getType() == EMPTY) {
                        if (fy == promoteRank) {
                            for (Figures promo : {QUEEN, ROOK, BISHOP, KNIGHT})
                                pseudo.push_back({x, y, x, fy, promo});
                        } else {
                            pseudo.push_back({x, y, x, fy, EMPTY});
                        }

                        // вперед 2 (только со старта и если промежуток пуст)
                        int fy2 = y + 2 * dir;
                        if (y == startRank && inBoard(x, fy2) &&
                            getPiece(x, fy2).getType() == EMPTY && getPiece(x, fy).getType() == EMPTY) {
                            pseudo.push_back({x, y, x, fy2, EMPTY});
                        }
                    }

                    // взятия по диагонали
                    for (int dx : {-1, 1}) {
                        int nx = x + dx, ny = y + dir;
                        if (!inBoard(nx, ny)) continue;
                        Piece target = getPiece(nx, ny);
                        if (target.getType() != EMPTY && target.isWhite() != white) {
                            if (ny == promoteRank) {
                                for (Figures promo : {QUEEN, ROOK, BISHOP, KNIGHT})
                                    pseudo.push_back({x, y, nx, ny, promo});
                            } else {
                                pseudo.push_back({x, y, nx, ny, EMPTY});
                            }
                        }
                    }

                    // en-passant (если ep хранит координату клетки, на которую пойдет побившая пешка)
                    auto ep = getEnPassant();
                    if (ep.first != -1 && ep.second != -1) {
                        int nx = ep.first, ny = ep.second;
                        if (ny == y + dir && std::abs(nx - x) == 1) {
                            Move m{x, y, nx, ny, EMPTY};
                            m.isEnPassant = true;
                            pseudo.push_back(m);
                        }
                    }
                    break;
                }

                case KNIGHT: {
                    static const int kdx[8] = {1,2,2,1,-1,-2,-2,-1};
                    static const int kdy[8] = {2,1,-1,-2,-2,-1,1,2};
                    for (int i = 0; i < 8; ++i) {
                        int nx = x + kdx[i], ny = y + kdy[i];
                        if (!inBoard(nx, ny)) continue;
                        Piece t = getPiece(nx, ny);
                        if (t.getType() == EMPTY || t.isWhite() != white)
                            pseudo.push_back({x, y, nx, ny, EMPTY});
                    }
                    break;
                }

                case BISHOP:
                case ROOK:
                case QUEEN: {
                    static const int dirs[8][2] = {
                        {1,0},{-1,0},{0,1},{0,-1},
                        {1,1},{1,-1},{-1,1},{-1,-1}
                    };
                    int i0 = 0, i1 = 8;
                    if (p.getType() == ROOK) { i0 = 0; i1 = 4; }
                    else if (p.getType() == BISHOP) { i0 = 4; i1 = 8; }
                    for (int i = i0; i < i1; ++i) {
                        int dx = dirs[i][0], dy = dirs[i][1];
                        int nx = x + dx, ny = y + dy;
                        while (inBoard(nx, ny)) {
                            Piece t = getPiece(nx, ny);
                            if (t.getType() == EMPTY) {
                                pseudo.push_back({x, y, nx, ny, EMPTY});
                            } else {
                                if (t.isWhite() != white)
                                    pseudo.push_back({x, y, nx, ny, EMPTY});
                                break;
                            }
                            nx += dx; ny += dy;
                        }
                    }
                    break;
                }

                case KING: {
                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = x + dx, ny = y + dy;
                            if (!inBoard(nx, ny)) continue;
                            Piece t = getPiece(nx, ny);
                            if (t.getType() == EMPTY || t.isWhite() != white)
                                pseudo.push_back({x, y, nx, ny, EMPTY});
                        }
                    }

                    genCastling(*this, white, x, y, pseudo);
                    break;
                }
                default: break;
            } // switch
        } // for y
    } // for x

    // Фильтрация: оставляем только ходы, которые НЕ оставляют короля под шахом
    std::vector<Move> legal;
    legal.reserve(pseudo.size());
    for (const auto &m : pseudo) {
        Position copy = *this;
        bool moverWasWhite = copy.isWhiteToMove();

        // применяем ход (applyMove у тебя уже переключает сторону внутри)
        copy.applyMove(m);

        // вернуть сторону к ходившему, чтобы isCheck проверял короля ходившего цвета
        copy.setIsWhiteMove(moverWasWhite);
        if (!copy.isCheck()) {
            legal.push_back(m);
        }
        // (не нужно восстанавливать copy дальше — он временный)
    }

    return legal;
}