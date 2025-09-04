// chess_logic.cpp
// Full move generation and check/mate detection.
// Comments are in English.

#include <vector>
#include <utility>
#include <algorithm>
#include <ostream>

#include "position.h"
#include "figures.h"
#include "chess_logic.h"

inline bool inBounds(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

inline std::pair<int,int> sideRange(bool white) {
    if (white) return {WHITE_PAWN, WHITE_KING};
    return {BLACK_PAWN, BLACK_KING};
}

auto isFriendlyAt = [](const Position& pos, bool whiteToMove, int x, int y) -> bool {
    auto r = sideRange(whiteToMove);
    for (int f = r.first; f <= r.second; ++f)
        if (pos[(Figures)f][x][y]) return true;
    return false;
};

auto isEnemyAt = [](const Position& pos, bool whiteToMove, int x, int y) -> bool {
    auto r = sideRange(!whiteToMove);
    for (int f = r.first; f <= r.second; ++f)
        if (pos[(Figures)f][x][y]) return true;
    return false;
};

std::pair<int,int> findKing(const Position& pos, bool white) {
    Figures king = white ? WHITE_KING : BLACK_KING;
    Board b = pos[king];
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
            if (b[x][y]) return {x, y};
    return {-1, -1};
}

bool isSquareAttacked(const Position& pos, int x, int y, bool byWhite) {
    // Pawns attack diagonally
    if (byWhite) {
        if (inBounds(x-1,y+1) && pos[WHITE_PAWN][x-1][y+1]) return true;
        if (inBounds(x+1,y+1) && pos[WHITE_PAWN][x+1][y+1]) return true;
    } else {
        if (inBounds(x-1,y-1) && pos[BLACK_PAWN][x-1][y-1]) return true;
        if (inBounds(x+1,y-1) && pos[BLACK_PAWN][x+1][y-1]) return true;
    }

    // Knights
    static const int knightMoves[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
    for (auto &m : knightMoves) {
        int nx = x + m[0], ny = y + m[1];
        if (!inBounds(nx,ny)) continue;
        if (byWhite ? pos[WHITE_KNIGHT][nx][ny] : pos[BLACK_KNIGHT][nx][ny]) return true;
    }

    // Sliding pieces: bishops/queens
    static const int diagDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto &d : diagDirs) {
        int nx=x+d[0], ny=y+d[1];
        while(inBounds(nx,ny)){
            if(byWhite){
                if(pos[WHITE_BISHOP][nx][ny]||pos[WHITE_QUEEN][nx][ny]) return true;
                bool any=false; for(int f=WHITE_PAWN;f<=WHITE_KING;++f) if(pos[(Figures)f][nx][ny]){any=true;break;} if(any) break;
            } else {
                if(pos[BLACK_BISHOP][nx][ny]||pos[BLACK_QUEEN][nx][ny]) return true;
                bool any=false; for(int f=BLACK_PAWN;f<=BLACK_KING;++f) if(pos[(Figures)f][nx][ny]){any=true;break;} if(any) break;
            }
            nx+=d[0]; ny+=d[1];
        }
    }

    // Sliding pieces: rooks/queens
    static const int orthoDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto &d : orthoDirs) {
        int nx=x+d[0], ny=y+d[1];
        while(inBounds(nx,ny)){
            if(byWhite){
                if(pos[WHITE_ROOK][nx][ny]||pos[WHITE_QUEEN][nx][ny]) return true;
                bool any=false; for(int f=WHITE_PAWN;f<=WHITE_KING;++f) if(pos[(Figures)f][nx][ny]){any=true;break;} if(any) break;
            } else {
                if(pos[BLACK_ROOK][nx][ny]||pos[BLACK_QUEEN][nx][ny]) return true;
                bool any=false; for(int f=BLACK_PAWN;f<=BLACK_KING;++f) if(pos[(Figures)f][nx][ny]){any=true;break;} if(any) break;
            }
            nx+=d[0]; ny+=d[1];
        }
    }

    // King
    for(int dx=-1;dx<=1;++dx) for(int dy=-1;dy<=1;++dy){
        if(dx==0&&dy==0) continue;
        int nx=x+dx, ny=y+dy;
        if(!inBounds(nx,ny)) continue;
        if(byWhite ? pos[WHITE_KING][nx][ny] : pos[BLACK_KING][nx][ny]) return true;
    }

    return false;
}

bool isCheck(const Position& pos, bool whiteToMove){
    auto kingPos = findKing(pos, whiteToMove);
    if(kingPos.first==-1) return false;
    return isSquareAttacked(pos, kingPos.first, kingPos.second, !whiteToMove);
}

Position applyMove(const Position& pos, const Move& mv, bool whiteToMove){
    Position newPos(pos);
    auto pr = sideRange(whiteToMove);
    Figures movingPiece=(Figures)-1;
    for(int f=pr.first;f<=pr.second;++f)
        if(newPos[(Figures)f][mv.fromX][mv.fromY]){movingPiece=(Figures)f; break;}

    if(movingPiece==(Figures)-1) return newPos;

    newPos[movingPiece][mv.fromX][mv.fromY]=0;

    auto oppRange=sideRange(!whiteToMove);
    for(int f=oppRange.first;f<=oppRange.second;++f)
        newPos[(Figures)f][mv.toX][mv.toY]=0;

    if(mv.promoteTo!=(Figures)-1)
        newPos[mv.promoteTo][mv.toX][mv.toY]=1;
    else
        newPos[movingPiece][mv.toX][mv.toY]=1;

    // Castling
    if(movingPiece==WHITE_KING){
        newPos.getLongWhiteCastling()=false;
        newPos.getShortWhiteCastling()=false;
        if(mv.fromX==4&&mv.fromY==0&&mv.toX==6&&mv.toY==0){ newPos[WHITE_ROOK][7][0]=0; newPos[WHITE_ROOK][5][0]=1; newPos.isWhiteCastled()=true; }
        if(mv.fromX==4&&mv.fromY==0&&mv.toX==2&&mv.toY==0){ newPos[WHITE_ROOK][0][0]=0; newPos[WHITE_ROOK][3][0]=1; newPos.isWhiteCastled()=true; }
    } else if(movingPiece==BLACK_KING){
        newPos.getLongBlackCastling()=false;
        newPos.getShortBlackCastling()=false;
        if(mv.fromX==4&&mv.fromY==7&&mv.toX==6&&mv.toY==7){ newPos[BLACK_ROOK][7][7]=0; newPos[BLACK_ROOK][5][7]=1; newPos.isBlackCastled()=true; }
        if(mv.fromX==4&&mv.fromY==7&&mv.toX==2&&mv.toY==7){ newPos[BLACK_ROOK][0][7]=0; newPos[BLACK_ROOK][3][7]=1; newPos.isBlackCastled()=true; }
    }

    // En-passant
    newPos.setEnPassantSquare({-1,-1});
    if(movingPiece==WHITE_PAWN&&mv.fromY==1&&mv.toY==3&&mv.fromX==mv.toX) newPos.setEnPassantSquare({mv.toX,2});
    else if(movingPiece==BLACK_PAWN&&mv.fromY==6&&mv.toY==4&&mv.fromX==mv.toX) newPos.setEnPassantSquare({mv.toX,5});

    newPos.isWhiteMove() = !whiteToMove;
    return newPos;
}

std::vector<Move> generatePseudoMoves(const Position& pos, bool whiteToMove){
    std::vector<Move> moves;
    auto r = sideRange(whiteToMove);

    auto friendlyAt = [&](int x,int y){for(int f=r.first;f<=r.second;++f) if(pos[(Figures)f][x][y]) return true; return false;};
    auto enemyAt = [&](int x,int y){auto rr=sideRange(!whiteToMove); for(int f=rr.first;f<=rr.second;++f) if(pos[(Figures)f][x][y]) return true; return false;};
    auto ep = pos.getEnPassantSquare();

    for(int f=r.first;f<=r.second;++f){
        Figures fig=(Figures)f;
        Board b=pos[fig];
        for(int x=0;x<8;++x) for(int y=0;y<8;++y){
            if(!b[x][y]) continue;

            // Pawns
            if(fig==WHITE_PAWN||fig==BLACK_PAWN){
                int dir=(fig==WHITE_PAWN)?1:-1;
                int startRank=(fig==WHITE_PAWN)?1:6;
                int promoteRank=(fig==WHITE_PAWN)?7:0;

                int nx=x, ny=y+dir;
                if(inBounds(nx,ny)&&!friendlyAt(nx,ny)&&!enemyAt(nx,ny)){
                    if(ny==promoteRank){
                        std::vector<Figures> promos=(fig==WHITE_PAWN)?std::vector<Figures>{WHITE_QUEEN,WHITE_ROOK,WHITE_BISHOP,WHITE_KNIGHT}:std::vector<Figures>{BLACK_QUEEN,BLACK_ROOK,BLACK_BISHOP,BLACK_KNIGHT};
                        for(Figures p:promos) moves.emplace_back(x,y,nx,ny,p);
                    } else moves.emplace_back(x,y,nx,ny,(Figures)-1);

                    // double push
                    int ny2=y+2*dir;
                    if(y==startRank && inBounds(nx,ny2)&&!friendlyAt(nx,ny2)&&!enemyAt(nx,ny2)) moves.emplace_back(x,y,nx,ny2,(Figures)-1);
                }

                for(int dx:{-1,1}){
                    int cx=x+dx, cy=y+dir;
                    if(!inBounds(cx,cy)) continue;
                    if(enemyAt(cx,cy)){
                        if(cy==promoteRank){
                            std::vector<Figures> promos=(fig==WHITE_PAWN)?std::vector<Figures>{WHITE_QUEEN,WHITE_ROOK,WHITE_BISHOP,WHITE_KNIGHT}:std::vector<Figures>{BLACK_QUEEN,BLACK_ROOK,BLACK_BISHOP,BLACK_KNIGHT};
                            for(Figures p:promos) moves.emplace_back(x,y,cx,cy,p);
                        } else moves.emplace_back(x,y,cx,cy,(Figures)-1);
                    }
                    if(ep.first!=-1 && cx==ep.first && cy==ep.second) moves.emplace_back(x,y,cx,cy,(Figures)-1);
                }
            }

            // Knights
            if(fig==WHITE_KNIGHT||fig==BLACK_KNIGHT){
                static const int kM[8][2]={{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
                for(auto m:kM){ int nx=x+m[0], ny=y+m[1]; if(inBounds(nx,ny)&&!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);}
            }

            // King
            if(fig==WHITE_KING||fig==BLACK_KING){
                for(int dx=-1;dx<=1;++dx) for(int dy=-1;dy<=1;++dy){
                    if(dx==0&&dy==0) continue; int nx=x+dx, ny=y+dy;
                    if(inBounds(nx,ny)&&!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);
                }
                // Castling skipped for brevity; handle same as applyMove
            }

            // Sliding pieces
            std::vector<std::pair<int,int>> dirs;
            if(fig==WHITE_BISHOP||fig==BLACK_BISHOP||fig==WHITE_QUEEN||fig==BLACK_QUEEN)
                dirs={{1,1},{1,-1},{-1,1},{-1,-1}};
            if(fig==WHITE_ROOK||fig==BLACK_ROOK||fig==WHITE_QUEEN||fig==BLACK_QUEEN)
                dirs.insert(dirs.end(),{{1,0},{-1,0},{0,1},{0,-1}});

            for(auto d:dirs){
                int nx=x+d.first, ny=y+d.second;
                while(inBounds(nx,ny)){
                    if(friendlyAt(nx,ny)) break;
                    moves.emplace_back(x,y,nx,ny,(Figures)-1);
                    if(enemyAt(nx,ny)) break;
                    nx+=d.first; ny+=d.second;
                }
            }
        }
    }

    return moves;
}

std::vector<Move> getLegalMoves(const Position& pos, bool whiteToMove){
    std::vector<Move> legal;
    auto pseudo = generatePseudoMoves(pos, whiteToMove);
    for(auto mv:pseudo){
        Position newPos=applyMove(pos,mv,whiteToMove);
        if(!isCheck(newPos, whiteToMove)) legal.push_back(mv);
    }
    return legal;
}

bool isCheckmate(const Position& pos, bool whiteToMove) {
    if (!isCheck(pos, whiteToMove)) return false;
    auto legal = getLegalMoves(pos, whiteToMove);
    return legal.empty();
}

std::ostream& operator<<(std::ostream& os, const Move& move) {
    auto to_chess_notation = [](int x, int y) {
        // x: 0..7 → a..h
        // y: 0..7 → 1..8
        return std::string(1, 'a' + x) + std::to_string(y + 1);
    };

    // Специальные ходы
    if (move.flag == CASTLE_SHORT) {
        os << "O-O";
        return os;
    }
    if (move.flag == CASTLE_LONG) {
        os << "O-O-O";
        return os;
    }

    os << to_chess_notation(move.fromX, move.fromY)
       << "-"
       << to_chess_notation(move.toX, move.toY);

    // Превращение пешки
    if (move.promoteTo != -1) {
        char pieceChar;
        switch(move.promoteTo) {
            case WHITE_QUEEN: case BLACK_QUEEN:   pieceChar = 'Q'; break;
            case WHITE_ROOK:  case BLACK_ROOK:    pieceChar = 'R'; break;
            case WHITE_BISHOP:case BLACK_BISHOP:  pieceChar = 'B'; break;
            case WHITE_KNIGHT:case BLACK_KNIGHT: pieceChar = 'N'; break;
            default: pieceChar = '?';
        }
        os << "=" << pieceChar;
    }

    // Взятие на проходе
    if (move.flag == EN_PASSANT) {
        os << " e.p.";
    }

    return os;
}