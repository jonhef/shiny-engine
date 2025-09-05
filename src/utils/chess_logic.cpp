// chess_logic.cpp
// Оптимизированная версия с акцентом на скорость (без изменения API Position/Board)
#include <vector>
#include <array>
#include <utility>
#include <ostream>
#include "position.h"
#include "figures.h"
#include "chess_logic.h"
#include <iostream>

// --- Небольшие константы и утилиты
inline bool inBounds(int x, int y) { return (unsigned)x < 8 && (unsigned)y < 8; }

bool Move::operator==(const Move& other) const {
    return fromX == other.fromX && fromY == other.fromY &&
           toX == other.toX && toY == other.toY &&
           promoteTo == other.promoteTo &&
           flag == other.flag;
}

static const std::array<Figures,6> WHITE_PIECES = {
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING
};
static const std::array<Figures,6> BLACK_PIECES = {
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
};

// --- Быстрые запросы по доске (используются внутри "горячих" функций).
//   Предполагается, что pos[f][x][y] — быстрый доступ (например, bool[][]).
inline bool anyPieceAt_raw(const Position& pos, int x, int y) {
    if (!inBounds(x,y)) return false;
    for (auto p : WHITE_PIECES) if (pos[p][x][y]) return true;
    for (auto p : BLACK_PIECES) if (pos[p][x][y]) return true;
    return false;
}

// findKing — оптимизирован: ранний выход, возврат {x,y}
std::pair<int,int> findKing(const Position& pos, bool white) {
    Figures king = white ? WHITE_KING : BLACK_KING;
    const Board &b = pos[king];
    for (int x = 0; x < 8; ++x) {
        // проходим по y внутри цикла x — обычно локальнее в памяти, зависит от Board layout
        for (int y = 0; y < 8; ++y) {
            if (b[x][y]) return {x, y};
        }
    }
    return {-1, -1};
}

// isSquareAttacked: оптимизирована путём локального доступа и минимизации вызовов anyPieceAt.
bool isSquareAttacked(const Position& pos, int x, int y, bool byWhite) {
    if (!inBounds(x,y)) return false;

    // Пешки: проверяем только две возможные диагонали
    int pd = byWhite ? 1 : -1;
    int ay = y + pd;
    if ((unsigned)ay < 8) {
        int lx = x - 1;
        if ((unsigned)lx < 8) {
            if (byWhite ? pos[WHITE_PAWN][lx][ay] : pos[BLACK_PAWN][lx][ay]) return true;
        }
        int rx = x + 1;
        if ((unsigned)rx < 8) {
            if (byWhite ? pos[WHITE_PAWN][rx][ay] : pos[BLACK_PAWN][rx][ay]) return true;
        }
    }

    // Кони
    static const int km[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
    for (int i = 0; i < 8; ++i) {
        int nx = x + km[i][0], ny = y + km[i][1];
        if ((unsigned)nx < 8 && (unsigned)ny < 8) {
            if (byWhite ? pos[WHITE_KNIGHT][nx][ny] : pos[BLACK_KNIGHT][nx][ny]) return true;
        }
    }

    // Диагонали: bishop/queen
    static const int diag[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (int d = 0; d < 4; ++d) {
        int dx = diag[d][0], dy = diag[d][1];
        int nx = x + dx, ny = y + dy;
        while ((unsigned)nx < 8 && (unsigned)ny < 8) {
            if (byWhite) {
                if (pos[WHITE_BISHOP][nx][ny] || pos[WHITE_QUEEN][nx][ny]) return true;
            } else {
                if (pos[BLACK_BISHOP][nx][ny] || pos[BLACK_QUEEN][nx][ny]) return true;
            }
            // если любая фигура стоит — дальше по этому направлению нет смысла
            if (anyPieceAt_raw(pos, nx, ny)) break;
            nx += dx; ny += dy;
        }
    }

    // Ортогонали: rook/queen
    static const int ortho[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (int d = 0; d < 4; ++d) {
        int dx = ortho[d][0], dy = ortho[d][1];
        int nx = x + dx, ny = y + dy;
        while ((unsigned)nx < 8 && (unsigned)ny < 8) {
            if (byWhite) {
                if (pos[WHITE_ROOK][nx][ny] || pos[WHITE_QUEEN][nx][ny]) return true;
            } else {
                if (pos[BLACK_ROOK][nx][ny] || pos[BLACK_QUEEN][nx][ny]) return true;
            }
            if (anyPieceAt_raw(pos, nx, ny)) break;
            nx += dx; ny += dy;
        }
    }

    // Король (смежные клетки)
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if ((unsigned)nx < 8 && (unsigned)ny < 8) {
                if (byWhite ? pos[WHITE_KING][nx][ny] : pos[BLACK_KING][nx][ny]) return true;
            }
        }
    }

    return false;
}

// isCheck: проверяем, под шахом ли король стороны, которая ходит (текущая сторона pos.isWhiteMove())
bool isCheck(const Position& pos) {
    bool whiteToMove = pos.isWhiteMove();
    auto k = findKing(pos, whiteToMove);
    if (k.first == -1) return false;
    // атакует ли соперник короля стороны, которая ходит?
    return isSquareAttacked(pos, k.first, k.second, !whiteToMove);
}

// applyMove: сохраним текущую логику, но минимизируем лишние операции
Position applyMove(const Position& pos, const Move& mv) {
    Position newPos(pos); // предполагаем, что копирование Position — себестоимость, зависящая от реализации
    bool whiteToMove = pos.isWhiteMove();

    const auto& movers = whiteToMove ? WHITE_PIECES : BLACK_PIECES;
    const auto& opponents = whiteToMove ? BLACK_PIECES : WHITE_PIECES;

    // Найдём движущуюся фигуру
    Figures movingPiece = (Figures)-1;
    for (auto f : movers) {
        if (pos[f][mv.fromX][mv.fromY]) { movingPiece = f; break; }
    }
    if (movingPiece == (Figures)-1) return newPos;

    // переключаем сторону
    newPos.isWhiteMove() = !whiteToMove;

    // убираем фигуру с from
    newPos[movingPiece][mv.fromX][mv.fromY] = 0;

    // удаляем фигуру на to (обычное взятие)
    for (auto f : opponents) newPos[f][mv.toX][mv.toY] = 0;

    // en-passant: если ход совпадает с ep — удаляем захваченную пешку
    auto ep = pos.getEnPassantSquare();
    if ((movingPiece == WHITE_PAWN || movingPiece == BLACK_PAWN) && ep.first != -1) {
        if (mv.toX == ep.first && mv.toY == ep.second) {
            int capY = ep.second + ((movingPiece == WHITE_PAWN) ? -1 : 1);
            if (inBounds(ep.first, capY)) {
                auto capturedPawn = (whiteToMove ? BLACK_PAWN : WHITE_PAWN);
                newPos[capturedPawn][ep.first][capY] = 0;
            }
        }
    }

    // Рокировка: двигаем ладью корректно при флагах
    if (mv.flag == CASTLE_SHORT) {
        int rookFromX = 7, rookToX = 5, y = mv.toY;
        auto rook = whiteToMove ? WHITE_ROOK : BLACK_ROOK;
        newPos[rook][rookFromX][y] = 0;
        newPos[rook][rookToX][y] = 1;
    } else if (mv.flag == CASTLE_LONG) {
        int rookFromX = 0, rookToX = 3, y = mv.toY;
        auto rook = whiteToMove ? WHITE_ROOK : BLACK_ROOK;
        newPos[rook][rookFromX][y] = 0;
        newPos[rook][rookToX][y] = 1;
    }

    // Разместим движущуюся фигуру или промоцию
    if (mv.promoteTo != (Figures)-1) {
        newPos[mv.promoteTo][mv.toX][mv.toY] = 1;
    } else {
        newPos[movingPiece][mv.toX][mv.toY] = 1;
    }

    // Обновление en-passant: сброс, либо установить при двойном ходе пешки
    newPos.setEnPassantSquare({-1, -1});
    if (movingPiece == WHITE_PAWN) {
        if (mv.fromY == 1 && mv.toY == 3 && mv.fromX == mv.toX)
            newPos.setEnPassantSquare({mv.toX, 2});
    } else if (movingPiece == BLACK_PAWN) {
        if (mv.fromY == 6 && mv.toY == 4 && mv.fromX == mv.toX)
            newPos.setEnPassantSquare({mv.toX, 5});
    }

    // Примечание: права на рокировку (если они есть в Position) должны обновляться извне или в Position.
    return newPos;
}

// generatePseudoMoves: горячая функция — максимальная локализация, минимальные вызовы
std::vector<Move> generatePseudoMoves(const Position& pos) {
    // Оценка среднего количества ходов: резервируем немного больше, чтобы избежать ресайзов
    std::vector<Move> moves;
    moves.reserve(128);

    bool whiteToMove = pos.isWhiteMove();
    const auto& movers = whiteToMove ? WHITE_PIECES : BLACK_PIECES;
    auto ep = pos.getEnPassantSquare();

    // Локальные карты occupancy: friendly[x][y], enemy[x][y], any[x][y]
    bool friendly[8][8] = {};
    bool enemy[8][8] = {};
    bool occ[8][8] = {};

    // Заполним карты один раз (избегаем многократных сканов)
    if (whiteToMove) {
        for (auto p : WHITE_PIECES) {
            const Board &b = pos[p];
            for (int x = 0; x < 8; ++x) for (int y = 0; y < 8; ++y) {
                if (b[x][y]) { friendly[x][y] = true; occ[x][y] = true; }
            }
        }
        for (auto p : BLACK_PIECES) {
            const Board &b = pos[p];
            for (int x = 0; x < 8; ++x) for (int y = 0; y < 8; ++y) {
                if (b[x][y]) { enemy[x][y] = true; occ[x][y] = true; }
            }
        }
    } else {
        for (auto p : BLACK_PIECES) {
            const Board &b = pos[p];
            for (int x = 0; x < 8; ++x) for (int y = 0; y < 8; ++y) {
                if (b[x][y]) { friendly[x][y] = true; occ[x][y] = true; }
            }
        }
        for (auto p : WHITE_PIECES) {
            const Board &b = pos[p];
            for (int x = 0; x < 8; ++x) for (int y = 0; y < 8; ++y) {
                if (b[x][y]) { enemy[x][y] = true; occ[x][y] = true; }
            }
        }
    }

    // Локальные лямбды для быстрого доступа (inline)
    auto friendlyAt = [&](int x, int y) -> bool { return (unsigned)x < 8 && (unsigned)y < 8 && friendly[x][y]; };
    auto enemyAt    = [&](int x, int y) -> bool { return (unsigned)x < 8 && (unsigned)y < 8 && enemy[x][y]; };
    auto anyAt      = [&](int x, int y) -> bool { return (unsigned)x < 8 && (unsigned)y < 8 && occ[x][y]; };

    // Перебираем все фигуры движущейся стороны
    for (auto fig : movers) {
        const Board &b = pos[fig];
        for (int x = 0; x < 8; ++x) {
            for (int y = 0; y < 8; ++y) {
                if (!b[x][y]) continue;

                // ---- ПЕШКИ ----
                if (fig == WHITE_PAWN || fig == BLACK_PAWN) {
                    int dir = (fig == WHITE_PAWN) ? 1 : -1;
                    int startY = (fig == WHITE_PAWN) ? 1 : 6;
                    int promoteY = (fig == WHITE_PAWN) ? 7 : 0;
                    int ny = y + dir;

                    // Вперед на 1 (пусто)
                    if ((unsigned)ny < 8 && !friendlyAt(x, ny) && !enemyAt(x, ny)) {
                        if (ny == promoteY) {
                            // промоции (четыре)
                            moves.emplace_back(x, y, x, ny, (fig == WHITE_PAWN) ? WHITE_QUEEN : BLACK_QUEEN);
                            moves.emplace_back(x, y, x, ny, (fig == WHITE_PAWN) ? WHITE_ROOK  : BLACK_ROOK);
                            moves.emplace_back(x, y, x, ny, (fig == WHITE_PAWN) ? WHITE_BISHOP: BLACK_BISHOP);
                            moves.emplace_back(x, y, x, ny, (fig == WHITE_PAWN) ? WHITE_KNIGHT: BLACK_KNIGHT);
                        } else {
                            moves.emplace_back(x, y, x, ny, (Figures)-1);
                        }

                        // Два шага с начальной позиции (проверяем промежуточную клетку)
                        if (y == startY) {
                            int ny2 = y + 2*dir;
                            if ((unsigned)ny2 < 8 && !anyAt(x, ny) && !anyAt(x, ny2)) {
                                moves.emplace_back(x, y, x, ny2, (Figures)-1);
                            }
                        }
                    }

                    // Взятия по диагонали и en-passant
                    for (int dx = -1; dx <= 1; dx += 2) {
                        int cx = x + dx, cy = y + dir;
                        if (!inBounds(cx, cy)) continue;
                        if (enemyAt(cx, cy)) {
                            if (cy == promoteY) {
                                moves.emplace_back(x, y, cx, cy, (fig == WHITE_PAWN) ? WHITE_QUEEN : BLACK_QUEEN);
                                moves.emplace_back(x, y, cx, cy, (fig == WHITE_PAWN) ? WHITE_ROOK  : BLACK_ROOK);
                                moves.emplace_back(x, y, cx, cy, (fig == WHITE_PAWN) ? WHITE_BISHOP: BLACK_BISHOP);
                                moves.emplace_back(x, y, cx, cy, (fig == WHITE_PAWN) ? WHITE_KNIGHT: BLACK_KNIGHT);
                            } else {
                                moves.emplace_back(x, y, cx, cy, (Figures)-1);
                            }
                        }
                        // en-passant (ep задана в координатах)
                        if (ep.first != -1 && cx == ep.first && cy == ep.second) {
                            Move m(x, y, cx, cy, (Figures)-1);
                            m.flag = EN_PASSANT;
                            moves.push_back(m);
                        }
                    }

                    continue;
                }

                // ---- КОНЬ ----
                if (fig == WHITE_KNIGHT || fig == BLACK_KNIGHT) {
                    static const int kM[8][2] = {
                        {1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}
                    };
                    for (int i = 0; i < 8; ++i) {
                        int nx = x + kM[i][0], ny = y + kM[i][1];
                        if (!inBounds(nx, ny)) continue;
                        if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                    }
                    continue;
                }

                // ---- СЛОН ----
                if (fig == WHITE_BISHOP || fig == BLACK_BISHOP) {
                    static const int dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
                    for (int di = 0; di < 4; ++di) {
                        int dx = dirs[di][0], dy = dirs[di][1];
                        int nx = x + dx, ny = y + dy;
                        while ((unsigned)nx < 8 && (unsigned)ny < 8) {
                            if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                            if (friendlyAt(nx, ny) || enemyAt(nx, ny)) break;
                            nx += dx; ny += dy;
                        }
                    }
                    continue;
                }

                // ---- ЛАДЬЯ ----
                if (fig == WHITE_ROOK || fig == BLACK_ROOK) {
                    static const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                    for (int di = 0; di < 4; ++di) {
                        int dx = dirs[di][0], dy = dirs[di][1];
                        int nx = x + dx, ny = y + dy;
                        while ((unsigned)nx < 8 && (unsigned)ny < 8) {
                            if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                            if (friendlyAt(nx, ny) || enemyAt(nx, ny)) break;
                            nx += dx; ny += dy;
                        }
                    }
                    continue;
                }

                // ---- ФЕРЗЬ ----
                if (fig == WHITE_QUEEN || fig == BLACK_QUEEN) {
                    static const int dirs[8][2] = {
                        {1,0},{-1,0},{0,1},{0,-1},
                        {1,1},{1,-1},{-1,1},{-1,-1}
                    };
                    for (int di = 0; di < 8; ++di) {
                        int dx = dirs[di][0], dy = dirs[di][1];
                        int nx = x + dx, ny = y + dy;
                        while ((unsigned)nx < 8 && (unsigned)ny < 8) {
                            if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                            if (friendlyAt(nx, ny) || enemyAt(nx, ny)) break;
                            nx += dx; ny += dy;
                        }
                    }
                    continue;
                }

                // ---- КОРОЛЬ ----
                if (fig == WHITE_KING || fig == BLACK_KING) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            if (dx == 0 && dy == 0) continue;
                            int nx = x + dx, ny = y + dy;
                            if (!inBounds(nx, ny)) continue;
                            if (!friendlyAt(nx, ny)) moves.emplace_back(x, y, nx, ny, (Figures)-1);
                        }
                    }

                    // Рокировки (упрощённые проверки: пустые клетки + клетки не под атакой)
                    bool isWhite = (fig == WHITE_KING);
                    int yRow = isWhite ? 0 : 7;
                    if (x == 4 && y == yRow) {
                        // короткая: f=5,g=6 должны быть пусты и не под атакой
                        if (!anyAt(5, yRow) && !anyAt(6, yRow)) {
                            if (!isSquareAttacked(pos, 4, yRow, !isWhite) &&
                                !isSquareAttacked(pos, 5, yRow, !isWhite) &&
                                !isSquareAttacked(pos, 6, yRow, !isWhite)) {
                                // валидацию наличия ладьи на h(7) либо прав рокировки предполагаем на уровне Position
                                Move mv(4, yRow, 6, yRow, (Figures)-1);
                                mv.flag = CASTLE_SHORT;
                                moves.push_back(mv);
                            }
                        }
                        // длинная: b=1,c=2,d=3 пусты и клетки 4,3,2 не под атакой
                        if (!anyAt(1, yRow) && !anyAt(2, yRow) && !anyAt(3, yRow)) {
                            if (!isSquareAttacked(pos, 4, yRow, !isWhite) &&
                                !isSquareAttacked(pos, 3, yRow, !isWhite) &&
                                !isSquareAttacked(pos, 2, yRow, !isWhite)) {
                                Move mv(4, yRow, 2, yRow, (Figures)-1);
                                mv.flag = CASTLE_LONG;
                                moves.push_back(mv);
                            }
                        }
                    }
                }
            }
        }
    }

    return moves;
}

// getLegalMoves: фильтруем псевдоходы, удаляя те, что оставляют короля под шахом.
// Оптимизация: минимум копий, быстрые проверки isCheck(after)
std::vector<Move> getLegalMoves(const Position& pos) {
    std::vector<Move> legal;
    auto pseudo = generatePseudoMoves(pos);
    legal.reserve(pseudo.size());
    for (const Move &mv : pseudo) {
        Position after = applyMove(pos, mv);
        if (!isCheck(after)) legal.push_back(mv);
    }
    return legal;
}

// isCheckmate: быстрый путь — если нет легальных ходов и король под шахом.
bool isCheckmate(const Position& pos) {
    if (!isCheck(pos)) return false;
    auto legal = getLegalMoves(pos);
    return legal.empty();
}

// operator<< для Move (незначительная оптимизация в использовании локальной лямбды)
std::ostream& operator<<(std::ostream& os, const Move& move) {
    auto to_chess_notation = [&](int x, int y) -> std::string {
        return std::string(1, char('a' + x)) + std::to_string(y + 1);
    };
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
            default: break;
        }
        os << "=" << c;
    }
    if (move.flag == EN_PASSANT) os << " e.p.";
    return os;
}
