// chess_logic.cpp
#include <vector>
#include <utility>
#include <ostream>
#include "position.h"
#include "figures.h"
#include "chess_logic.h"

// Проверка границ доски
inline bool inBounds(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

// Диапазон фигур для стороны
inline std::pair<int,int> sideRange(bool white) {
    return white ? std::make_pair(WHITE_PAWN, WHITE_KING) : std::make_pair(BLACK_PAWN, BLACK_KING);
}

// Проверка на дружелюбную фигуру
auto isFriendlyAt = [](const Position& pos, bool whiteToMove, int x, int y) -> bool {
    auto r = sideRange(whiteToMove);
    for (int f = r.first; f <= r.second; ++f)
        if (pos[(Figures)f][x][y]) return true;
    return false;
};

// Проверка на вражескую фигуру
auto isEnemyAt = [](const Position& pos, bool whiteToMove, int x, int y) -> bool {
    auto r = sideRange(!whiteToMove);
    for (int f = r.first; f <= r.second; ++f)
        if (pos[(Figures)f][x][y]) return true;
    return false;
};

// Поиск короля
std::pair<int,int> findKing(const Position& pos, bool white) {
    Figures king = white ? WHITE_KING : BLACK_KING;
    Board b = pos[king];
    for (int x = 0; x < 8; ++x) for (int y = 0; y < 8; ++y)
        if (b[x][y]) return {x, y};
    return {-1, -1};
}

// Проверка атаки на клетку
bool isSquareAttacked(const Position& pos, int x, int y, bool byWhite) {
    int dir = byWhite ? 1 : -1;

    // Пешки
    if (inBounds(x-1, y+dir) && ((byWhite ? pos[WHITE_PAWN][x-1][y+dir] : pos[BLACK_PAWN][x-1][y+dir]))) return true;
    if (inBounds(x+1, y+dir) && ((byWhite ? pos[WHITE_PAWN][x+1][y+dir] : pos[BLACK_PAWN][x+1][y+dir]))) return true;

    // Кони
    static const int knightMoves[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
    for (auto &m : knightMoves) {
        int nx = x + m[0], ny = y + m[1];
        if (!inBounds(nx, ny)) continue;
        if (byWhite ? pos[WHITE_KNIGHT][nx][ny] : pos[BLACK_KNIGHT][nx][ny]) return true;
    }

    // Слон/ферзь по диагонали
    static const int diagDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto &d : diagDirs) {
        int nx=x+d[0], ny=y+d[1];
        while (inBounds(nx, ny)) {
            if (byWhite) {
                if (pos[WHITE_BISHOP][nx][ny] || pos[WHITE_QUEEN][nx][ny]) return true;
                bool any=false; for(int f=WHITE_PAWN; f<=WHITE_KING; ++f) if(pos[(Figures)f][nx][ny]){any=true; break;} if(any) break;
            } else {
                if (pos[BLACK_BISHOP][nx][ny] || pos[BLACK_QUEEN][nx][ny]) return true;
                bool any=false; for(int f=BLACK_PAWN; f<=BLACK_KING; ++f) if(pos[(Figures)f][nx][ny]){any=true; break;} if(any) break;
            }
            nx+=d[0]; ny+=d[1];
        }
    }

    // Ладья/ферзь по горизонтали/вертикали
    static const int orthoDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto &d : orthoDirs) {
        int nx=x+d[0], ny=y+d[1];
        while (inBounds(nx, ny)) {
            if (byWhite) {
                if (pos[WHITE_ROOK][nx][ny] || pos[WHITE_QUEEN][nx][ny]) return true;
                bool any=false; for(int f=WHITE_PAWN; f<=WHITE_KING; ++f) if(pos[(Figures)f][nx][ny]){any=true; break;} if(any) break;
            } else {
                if (pos[BLACK_ROOK][nx][ny] || pos[BLACK_QUEEN][nx][ny]) return true;
                bool any=false; for(int f=BLACK_PAWN; f<=BLACK_KING; ++f) if(pos[(Figures)f][nx][ny]){any=true; break;} if(any) break;
            }
            nx+=d[0]; ny+=d[1];
        }
    }

    // Король рядом
    for (int dx=-1; dx<=1; ++dx) for (int dy=-1; dy<=1; ++dy) {
        if (dx==0 && dy==0) continue;
        int nx=x+dx, ny=y+dy;
        if (!inBounds(nx, ny)) continue;
        if (byWhite ? pos[WHITE_KING][nx][ny] : pos[BLACK_KING][nx][ny]) return true;
    }

    return false;
}

// Проверка шаха
bool isCheck(const Position& pos, bool whiteToMove) {
    auto k = findKing(pos, whiteToMove);
    if (k.first==-1) return false;
    return isSquareAttacked(pos, k.first, k.second, !whiteToMove);
}

// Применение хода
Position applyMove(const Position& pos, const Move& mv, bool whiteToMove) {
    Position newPos(pos);
    auto pr = sideRange(whiteToMove);
    Figures movingPiece=(Figures)-1;
    for(int f=pr.first; f<=pr.second; ++f) if(newPos[(Figures)f][mv.fromX][mv.fromY]) {movingPiece=(Figures)f; break;}
    if(movingPiece==(Figures)-1) return newPos;

    newPos[movingPiece][mv.fromX][mv.fromY]=0;

    auto oppRange = sideRange(!whiteToMove);
    for(int f=oppRange.first; f<=oppRange.second; ++f) newPos[(Figures)f][mv.toX][mv.toY]=0;

    auto ep = newPos.getEnPassantSquare();
    if((movingPiece==WHITE_PAWN||movingPiece==BLACK_PAWN) && ep.first!=-1) {
        int capY=whiteToMove ? ep.second-1 : ep.second+1;
        if(mv.toX==ep.first && mv.toY==ep.second && inBounds(ep.first, capY))
            newPos[whiteToMove?BLACK_PAWN:WHITE_PAWN][ep.first][capY]=0;
    }

    if(mv.promoteTo!=(Figures)-1) newPos[mv.promoteTo][mv.toX][mv.toY]=1;
    else newPos[movingPiece][mv.toX][mv.toY]=1;

    newPos.isWhiteMove() = !whiteToMove;

    return newPos;
}

// Псевдозаконные ходы
std::vector<Move> generatePseudoMoves(const Position& pos, bool whiteToMove) {
    std::vector<Move> moves;
    auto r = sideRange(whiteToMove);
    auto ep = pos.getEnPassantSquare();

    auto friendlyAt = [&](int x,int y){for(int f=r.first;f<=r.second;++f) if(pos[(Figures)f][x][y]) return true; return false;};
    auto enemyAt = [&](int x,int y){auto rr=sideRange(!whiteToMove); for(int f=rr.first;f<=rr.second;++f) if(pos[(Figures)f][x][y]) return true; return false;};

    for(int f=r.first; f<=r.second; ++f){
        Figures fig=(Figures)f;
        Board b=pos[fig];
        for(int x=0;x<8;++x) for(int y=0;y<8;++y){
            if(!b[x][y]) continue;
            int dir=(fig==WHITE_PAWN?1:-1);
            int startY=(fig==WHITE_PAWN?1:6);
            int promoteY=(fig==WHITE_PAWN?7:0);

            if(fig==WHITE_PAWN||fig==BLACK_PAWN){
                int ny=y+dir;
                if(inBounds(x,ny)&&!friendlyAt(x,ny)&&!enemyAt(x,ny)){
                    if(ny==promoteY){
                        std::vector<Figures> promos={fig==WHITE_PAWN?WHITE_QUEEN:BLACK_QUEEN,
                                                     fig==WHITE_PAWN?WHITE_ROOK:BLACK_ROOK,
                                                     fig==WHITE_PAWN?WHITE_BISHOP:BLACK_BISHOP,
                                                     fig==WHITE_PAWN?WHITE_KNIGHT:BLACK_KNIGHT};
                        for(auto p:promos) moves.emplace_back(x,y,x,ny,p);
                    } else moves.emplace_back(x,y,x,ny,(Figures)-1);
                    if(y==startY){
                        int ny2=y+2*dir;
                        if(inBounds(x,ny2)&&!friendlyAt(x,ny2)&&!enemyAt(x,ny2)) moves.emplace_back(x,y,x,ny2,(Figures)-1);
                    }
                }
                for(int dx:{-1,1}){
                    int cx=x+dx, cy=y+dir;
                    if(!inBounds(cx,cy)) continue;
                    if(enemyAt(cx,cy)) moves.emplace_back(x,y,cx,cy,(Figures)-1);
                    if(ep.first!=-1 && cx==ep.first && cy==ep.second) moves.emplace_back(x,y,cx,cy,(Figures)-1);
                }
            }

            // Другие фигуры
            if(fig==WHITE_KNIGHT||fig==BLACK_KNIGHT){
                static const int kM[8][2]={{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
                for(auto &m:kM){int nx=x+m[0],ny=y+m[1]; if(!inBounds(nx,ny)) continue; if(!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);}
            }
            if(fig==WHITE_BISHOP||fig==BLACK_BISHOP){
                static const int dirs[4][2]={{1,1},{1,-1},{-1,1},{-1,-1}};
                for(auto &d:dirs){int nx=x+d[0],ny=y+d[1]; while(inBounds(nx,ny)){if(!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1); if(friendlyAt(nx,ny)||enemyAt(nx,ny)) break; nx+=d[0]; ny+=d[1];}}
            }
            if(fig==WHITE_ROOK||fig==BLACK_ROOK){
                static const int dirs[4][2]={{1,0},{-1,0},{0,1},{0,-1}};
                for(auto &d:dirs){int nx=x+d[0],ny=y+d[1]; while(inBounds(nx,ny)){if(!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1); if(friendlyAt(nx,ny)||enemyAt(nx,ny)) break; nx+=d[0]; ny+=d[1];}}
            }
            if(fig==WHITE_QUEEN||fig==BLACK_QUEEN){
                static const int dirs[8][2]={{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
                for(auto &d:dirs){int nx=x+d[0],ny=y+d[1]; while(inBounds(nx,ny)){if(!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1); if(friendlyAt(nx,ny)||enemyAt(nx,ny)) break; nx+=d[0]; ny+=d[1];}}
            }
            if(fig==WHITE_KING||fig==BLACK_KING){
                for(int dx=-1;dx<=1;++dx) for(int dy=-1;dy<=1;++dy){if(dx==0 && dy==0) continue; int nx=x+dx,ny=y+dy; if(!inBounds(nx,ny)) continue; if(!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);}
            }
        }
    }
    return moves;
}

// Легальные ходы
std::vector<Move> getLegalMoves(const Position& pos, bool whiteToMove){
    std::vector<Move> legal;
    auto pseudo=generatePseudoMoves(pos,whiteToMove);
    for(auto &mv:pseudo){
        Position after=applyMove(pos,mv,whiteToMove);
        if(isCheck(after,whiteToMove)) continue;
        legal.push_back(mv);
    }
    return legal;
}

// Проверка матовой позиции
bool isCheckmate(const Position& pos, bool whiteToMove){
    if(!isCheck(pos,whiteToMove)) return false;
    auto legal=getLegalMoves(pos,whiteToMove);
    return legal.empty();
}

// Вывод хода
std::ostream& operator<<(std::ostream& os, const Move& move){
    auto to_chess_notation=[&](int x,int y){return std::string(1,'a'+x)+std::to_string(y+1);};
    if(move.flag==CASTLE_SHORT){os<<"O-O"; return os;}
    if(move.flag==CASTLE_LONG){os<<"O-O-O"; return os;}
    os<<to_chess_notation(move.fromX,move.fromY)<<"-"<<to_chess_notation(move.toX,move.toY);
    if(move.promoteTo!=-1){
        char c='?';
        switch(move.promoteTo){
            case WHITE_QUEEN: case BLACK_QUEEN: c='Q'; break;
            case WHITE_ROOK: case BLACK_ROOK: c='R'; break;
            case WHITE_BISHOP: case BLACK_BISHOP: c='B'; break;
            case WHITE_KNIGHT: case BLACK_KNIGHT: c='N'; break;
        }
        os<<"="<<c;
    }
    if(move.flag==EN_PASSANT) os<<" e.p.";
    return os;
}
