// chess_logic.cpp
#include <vector>
#include <array>
#include <utility>
#include <ostream>
#include "position.h"
#include "figures.h"
#include "chess_logic.h"

// Проверка границ доски
inline bool inBounds(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

bool Move::operator==(const Move& other) const {
    return fromX == other.fromX && fromY == other.fromY &&
           toX == other.toX && toY == other.toY &&
            promoteTo == other.promoteTo;
}

// Явные списки фигур по цветам (используем значения из figures.h)
static const std::array<Figures,6> WHITE_PIECES = {
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING
};
static const std::array<Figures,6> BLACK_PIECES = {
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
};

// Проверяет, есть ли какая-либо фигура (любого цвета) на клетке
inline bool anyPieceAt(const Position& pos, int x, int y) {
    if (!inBounds(x,y)) return false;
    for (auto p : WHITE_PIECES) if (pos[p][x][y]) return true;
    for (auto p : BLACK_PIECES) if (pos[p][x][y]) return true;
    return false;
}

// Дружелюбная фигура (относительно текущего хода pos.isWhiteMove())
inline bool isFriendlyAt(const Position& pos, int x, int y) {
    if (!inBounds(x,y)) return false;
    if (pos.isWhiteMove()) {
        for (auto p : WHITE_PIECES) if (pos[p][x][y]) return true;
        return false;
    } else {
        for (auto p : BLACK_PIECES) if (pos[p][x][y]) return true;
        return false;
    }
}

// Вражеская фигура (относительно текущего хода pos.isWhiteMove())
inline bool isEnemyAt(const Position& pos, int x, int y) {
    if (!inBounds(x,y)) return false;
    if (pos.isWhiteMove()) {
        for (auto p : BLACK_PIECES) if (pos[p][x][y]) return true;
        return false;
    } else {
        for (auto p : WHITE_PIECES) if (pos[p][x][y]) return true;
        return false;
    }
}

// Поиск позиции короля заданного цвета
std::pair<int,int> findKing(const Position& pos, bool white) {
    Figures king = white ? WHITE_KING : BLACK_KING;
    const Board &b = pos[king];
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
            if (b[x][y]) return {x, y};
    return {-1, -1};
}

// Проверка, атакуется ли клетка (x,y) стороной byWhite
bool isSquareAttacked(const Position& pos, int x, int y, bool byWhite) {
    if (!inBounds(x,y)) return false;

    // Пешки (в твоей системе a1=[0][0], белые двигаются по +y)
    int pawnDir = byWhite ? 1 : -1;
    if (inBounds(x-1, y + pawnDir)) {
        if (byWhite ? pos[WHITE_PAWN][x-1][y + pawnDir] : pos[BLACK_PAWN][x-1][y + pawnDir]) return true;
    }
    if (inBounds(x+1, y + pawnDir)) {
        if (byWhite ? pos[WHITE_PAWN][x+1][y + pawnDir] : pos[BLACK_PAWN][x+1][y + pawnDir]) return true;
    }

    // Кони
    static const int knightMoves[8][2] = {
        {1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}
    };
    for (auto &m : knightMoves) {
        int nx = x + m[0], ny = y + m[1];
        if (!inBounds(nx, ny)) continue;
        if (byWhite ? pos[WHITE_KNIGHT][nx][ny] : pos[BLACK_KNIGHT][nx][ny]) return true;
    }

    // Диагонали (слон / ферзь)
    static const int diagDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto &d : diagDirs) {
        int nx = x + d[0], ny = y + d[1];
        while (inBounds(nx, ny)) {
            if (byWhite) {
                if (pos[WHITE_BISHOP][nx][ny] || pos[WHITE_QUEEN][nx][ny]) return true;
                if (anyPieceAt(pos, nx, ny)) break;
            } else {
                if (pos[BLACK_BISHOP][nx][ny] || pos[BLACK_QUEEN][nx][ny]) return true;
                if (anyPieceAt(pos, nx, ny)) break;
            }
            nx += d[0]; ny += d[1];
        }
    }

    // Ортогонали (ладья / ферзь)
    static const int orthoDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto &d : orthoDirs) {
        int nx = x + d[0], ny = y + d[1];
        while (inBounds(nx, ny)) {
            if (byWhite) {
                if (pos[WHITE_ROOK][nx][ny] || pos[WHITE_QUEEN][nx][ny]) return true;
                if (anyPieceAt(pos, nx, ny)) break;
            } else {
                if (pos[BLACK_ROOK][nx][ny] || pos[BLACK_QUEEN][nx][ny]) return true;
                if (anyPieceAt(pos, nx, ny)) break;
            }
            nx += d[0]; ny += d[1];
        }
    }

    // Король (смежные клетки)
    for (int dx=-1; dx<=1; ++dx) for (int dy=-1; dy<=1; ++dy) {
        if (dx==0 && dy==0) continue;
        int nx = x + dx, ny = y + dy;
        if (!inBounds(nx, ny)) continue;
        if (byWhite ? pos[WHITE_KING][nx][ny] : pos[BLACK_KING][nx][ny]) return true;
    }

    return false;
}

// Проверка шаха: проверяем, не находится ли в шахе король той стороны, которая только что ходила.
// Т.е. pos.isWhiteMove() — сторона, которая сейчас ходит; король, который только что ходил — !pos.isWhiteMove().
bool isCheck(const Position& pos) {
    bool whiteJustMoved = !pos.isWhiteMove();
    auto k = findKing(pos, whiteJustMoved);
    if (k.first == -1) return false;
    // Проверяем, атакует ли сейчас (ходящая) сторона короля, который только что ходил
    return isSquareAttacked(pos, k.first, k.second, pos.isWhiteMove());
}

// Применение хода: возвращает новую позицию (не проверяет легальность)
Position applyMove(const Position& pos, const Move& mv) {
    Position newPos(pos);

    bool whiteToMove = pos.isWhiteMove();                // сторона, которая делает ход сейчас (в pos)

    // Выбрать списки фигур для движущейся стороны и для соперника
    const auto& movers = whiteToMove ? WHITE_PIECES : BLACK_PIECES;
    const auto& opponents = whiteToMove ? BLACK_PIECES : WHITE_PIECES;

    Figures movingPiece = (Figures)-1;
    for (auto f : movers) {
        if (pos[f][mv.fromX][mv.fromY]) { movingPiece = f; break; }
    }
    if (movingPiece == (Figures)-1) return newPos;

    newPos.isWhiteMove() = !whiteToMove;

    // Удаляем её с исходной клетки
    newPos[movingPiece][mv.fromX][mv.fromY] = 0;

    // Удаляем любую фигуру соперника на целевой клетке
    for (auto f : opponents) newPos[f][mv.toX][mv.toY] = 0;

    // Обработка en-passant: берем ep из исходной позиции (pos), а не newPos
    auto ep = pos.getEnPassantSquare();
    if ((movingPiece == WHITE_PAWN || movingPiece == BLACK_PAWN) && ep.first != -1) {
        int dir = (movingPiece == WHITE_PAWN) ? 1 : -1;
        if (mv.toX == ep.first && mv.toY == ep.second) {
            int capY = ep.second - dir; // захваченная пешка находится "за" ep по направлению движущейся пешки
            if (inBounds(ep.first, capY)) {
                auto capturedPawn = (whiteToMove ? BLACK_PAWN : WHITE_PAWN);
                newPos[capturedPawn][ep.first][capY] = 0;
            }
        }
    }

    // Поставить движущуюся фигуру/промоцию на целевую клетку
    if (mv.promoteTo != (Figures)-1) {
        newPos[mv.promoteTo][mv.toX][mv.toY] = 1;
    } else {
        newPos[movingPiece][mv.toX][mv.toY] = 1;
    }

    // Обновим en-passant для newPos: по умолчанию — сброс
    newPos.setEnPassantSquare({-1, -1});
    // Если пешка сделала двойной ход — установить ep
    if (movingPiece == WHITE_PAWN) {
        if (mv.fromY == 1 && mv.toY == 3 && mv.fromX == mv.toX) {
            newPos.setEnPassantSquare({mv.toX, 2});
        }
    } else if (movingPiece == BLACK_PAWN) {
        if (mv.fromY == 6 && mv.toY == 4 && mv.fromX == mv.toX) {
            newPos.setEnPassantSquare({mv.toX, 5});
        }
    }

    // Примечание: права на рокировку не обновляются здесь. Можно добавить, если нужно.

    return newPos;
}

// Генерация всех псевдозаконных ходов для позиции pos (без проверки шаха)
std::vector<Move> generatePseudoMoves(const Position& pos) {
    std::vector<Move> moves;
    bool whiteToMove = pos.isWhiteMove();
    const auto& movers = whiteToMove ? WHITE_PIECES : BLACK_PIECES;
    auto ep = pos.getEnPassantSquare();

    auto friendlyAt = [&](int x, int y){ return isFriendlyAt(pos, x, y); };
    auto enemyAt    = [&](int x, int y){ return isEnemyAt(pos, x, y); };

    for (auto fig : movers) {
        Board b = pos[fig];
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                if (!b[x][y]) continue;

                // Пешки — явное разделение белых/чёрных по направлению
                if (fig == WHITE_PAWN || fig == BLACK_PAWN) {
                    int dir = (fig == WHITE_PAWN) ? 1 : -1;
                    int startY = (fig == WHITE_PAWN) ? 1 : 6;
                    int promoteY = (fig == WHITE_PAWN) ? 7 : 0;

                    int ny = y + dir;
                    // Одноклеточный ход вперёд
                    if (inBounds(x, ny) && !friendlyAt(x, ny) && !enemyAt(x, ny)) {
                        if (ny == promoteY) {
                            std::vector<Figures> promos = {
                                (fig == WHITE_PAWN) ? WHITE_QUEEN : BLACK_QUEEN,
                                (fig == WHITE_PAWN) ? WHITE_ROOK  : BLACK_ROOK,
                                (fig == WHITE_PAWN) ? WHITE_BISHOP: BLACK_BISHOP,
                                (fig == WHITE_PAWN) ? WHITE_KNIGHT: BLACK_KNIGHT
                            };
                            for (auto p : promos) moves.emplace_back(x, y, x, ny, p);
                        } else {
                            moves.emplace_back(x, y, x, ny, (Figures)-1);
                        }

                        // Двойной ход с начальной позиции
                        if (y == startY) {
                            int ny2 = y + 2 * dir;
                            if (inBounds(x, ny2) && !friendlyAt(x, ny2) && !enemyAt(x, ny2))
                                moves.emplace_back(x, y, x, ny2, (Figures)-1);
                        }
                    }

                    // Диагональные атаки
                    for (int dx : {-1, 1}) {
                        int cx = x + dx, cy = y + dir;
                        if (!inBounds(cx, cy)) continue;
                        if (enemyAt(cx, cy)) moves.emplace_back(x, y, cx, cy, (Figures)-1);
                        // En-passant: цель совпадает с ep
                        if (ep.first != -1 && cx == ep.first && cy == ep.second)
                            moves.emplace_back(x, y, cx, cy, (Figures)-1);
                    }
                    continue;
                }

                // Конь
                if (fig == WHITE_KNIGHT || fig == BLACK_KNIGHT) {
                    static const int kM[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
                    for (auto &m : kM) {
                        int nx = x + m[0], ny = y + m[1];
                        if (!inBounds(nx, ny)) continue;
                        if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                    }
                    continue;
                }

                // Слон
                if (fig == WHITE_BISHOP || fig == BLACK_BISHOP) {
                    static const int dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
                    for (auto &d : dirs) {
                        int nx = x + d[0], ny = y + d[1];
                        while (inBounds(nx, ny)) {
                            if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                            if (friendlyAt(nx, ny) || enemyAt(nx, ny)) break;
                            nx += d[0]; ny += d[1];
                        }
                    }
                    continue;
                }

                // Ладья
                if (fig == WHITE_ROOK || fig == BLACK_ROOK) {
                    static const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                    for (auto &d : dirs) {
                        int nx = x + d[0], ny = y + d[1];
                        while (inBounds(nx, ny)) {
                            if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                            if (friendlyAt(nx, ny) || enemyAt(nx, ny)) break;
                            nx += d[0]; ny += d[1];
                        }
                    }
                    continue;
                }

                // Ферзь
                if (fig == WHITE_QUEEN || fig == BLACK_QUEEN) {
                    static const int dirs[8][2] = {
                        {1,0},{-1,0},{0,1},{0,-1},
                        {1,1},{1,-1},{-1,1},{-1,-1}
                    };
                    for (auto &d : dirs) {
                        int nx = x + d[0], ny = y + d[1];
                        while (inBounds(nx, ny)) {
                            if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                            if (friendlyAt(nx, ny) || enemyAt(nx, ny)) break;
                            nx += d[0]; ny += d[1];
                        }
                    }
                    continue;
                }

                // Король
                if (fig == WHITE_KING || fig == BLACK_KING) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = x + dx, ny = y + dy;
                            if (!inBounds(nx, ny)) continue;
                            if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                        }
                    }
                    // Рокировка можно добавить при необходимости
                }
            }
        }
    }

    return moves;
}

// Легальные ходы: отфильтровать те, которые оставляют своего короля под шахом
std::vector<Move> getLegalMoves(const Position& pos) {
    std::vector<Move> legal;
    auto pseudo = generatePseudoMoves(pos);
    for (auto &mv : pseudo) {
        Position after = applyMove(pos, mv);
        if (isCheck(after)) continue; // если после хода король стороны, которая только что ходила, в шахе => ход нелегален
        legal.push_back(mv);
    }
    return legal;
}

// Проверка матовой позиции
bool isCheckmate(const Position& pos) {
    if (!isCheck(pos)) return false;
    auto legal = getLegalMoves(pos);
    return legal.empty();
}

// Вывод хода в простую нотацию
std::ostream& operator<<(std::ostream& os, const Move& move) {
    auto to_chess_notation = [&](int x, int y) { return std::string(1, 'a' + x) + std::to_string(y + 1); };
    if (move.flag == CASTLE_SHORT) { os << "O-O"; return os; }
    if (move.flag == CASTLE_LONG)  { os << "O-O-O"; return os; }
    os << to_chess_notation(move.fromX, move.fromY) << "-" << to_chess_notation(move.toX, move.toY);
    if (move.promoteTo != (Figures)-1) {
        char c = '?';
        switch (move.promoteTo) {
            case WHITE_QUEEN: case BLACK_QUEEN: c='Q'; break;
            case WHITE_ROOK:  case BLACK_ROOK:  c='R'; break;
            case WHITE_BISHOP:case BLACK_BISHOP:c='B'; break;
            case WHITE_KNIGHT:case BLACK_KNIGHT:c='N'; break;
        }
        os << "=" << c;
    }
    if (move.flag == EN_PASSANT) os << " e.p.";
    return os;
}
