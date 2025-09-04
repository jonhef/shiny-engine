#include "chess_logic.h"
#include <utility>
#include <cstring>

// ---------- Helpers ----------

inline bool inBounds(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

// Return true if any piece occupies (x,y)
static inline bool occupied(const Position& pos, int x, int y) {
    for (int f = WHITE_PAWN; f <= BLACK_KING; ++f)
        if (pos[(Figures)f][x][y]) return true;
    return false;
}

// Return true if a friendly piece (for 'white') occupies (x,y)
static inline bool friendlyAt(const Position& pos, bool white, int x, int y) {
    Figures s = white ? WHITE_PAWN : BLACK_PAWN;
    Figures e = white ? WHITE_KING : BLACK_KING;
    for (int f = s; f <= e; ++f)
        if (pos[(Figures)f][x][y]) return true;
    return false;
}

// Return true if an enemy piece (for 'white') occupies (x,y)
static inline bool enemyAt(const Position& pos, bool white, int x, int y) {
    Figures s = white ? BLACK_PAWN : WHITE_PAWN;
    Figures e = white ? BLACK_KING : WHITE_KING;
    for (int f = s; f <= e; ++f)
        if (pos[(Figures)f][x][y]) return true;
    return false;
}

// Find king square for the side 'white'
static std::pair<int,int> findKing(const Position& pos, bool white) {
    Figures king = white ? WHITE_KING : BLACK_KING;
    Board b = pos[king];
    for (int x=0;x<8;x++)
        for (int y=0;y<8;y++)
            if (b[x][y]) return {x,y};
    return {-1,-1};
}

// Check if (x,y) is attacked by side 'byWhite'
static bool isSquareAttacked(const Position& pos, int x, int y, bool byWhite) {
    // Pawn attacks
    if (byWhite) {
        if (inBounds(x-1,y-1) && pos[WHITE_PAWN][x-1][y-1]) return true;
        if (inBounds(x+1,y-1) && pos[WHITE_PAWN][x+1][y-1]) return true;
    } else {
        if (inBounds(x-1,y+1) && pos[BLACK_PAWN][x-1][y+1]) return true;
        if (inBounds(x+1,y+1) && pos[BLACK_PAWN][x+1][y+1]) return true;
    }

    // Knight attacks
    static const int KN[8][2] = {
        {1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}
    };
    for (auto& m : KN) {
        int nx = x + m[0], ny = y + m[1];
        if (inBounds(nx,ny) && pos[byWhite?WHITE_KNIGHT:BLACK_KNIGHT][nx][ny]) return true;
    }

    // Bishop/Queen diagonals
    static const int BD[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto& d : BD) {
        int nx = x + d[0], ny = y + d[1];
        while (inBounds(nx,ny)) {
            if (pos[byWhite?WHITE_BISHOP:BLACK_BISHOP][nx][ny] || pos[byWhite?WHITE_QUEEN:BLACK_QUEEN][nx][ny])
                return true;
            if (occupied(pos,nx,ny)) break;
            nx += d[0]; ny += d[1];
        }
    }

    // Rook/Queen orthogonals
    static const int RD[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto& d : RD) {
        int nx = x + d[0], ny = y + d[1];
        while (inBounds(nx,ny)) {
            if (pos[byWhite?WHITE_ROOK:BLACK_ROOK][nx][ny] || pos[byWhite?WHITE_QUEEN:BLACK_QUEEN][nx][ny])
                return true;
            if (occupied(pos,nx,ny)) break;
            nx += d[0]; ny += d[1];
        }
    }

    // King attacks
    for (int dx=-1; dx<=1; ++dx)
        for (int dy=-1; dy<=1; ++dy) {
            if (!dx && !dy) continue;
            int nx = x+dx, ny = y+dy;
            if (inBounds(nx,ny) && pos[byWhite?WHITE_KING:BLACK_KING][nx][ny]) return true;
        }

    return false;
}

// ---------- Core checks ----------

bool isCheck(const Position& pos, bool whiteToMove) {
    auto [kx,ky] = findKing(pos, whiteToMove);
    if (kx < 0) return false; // invalid position (no king found)
    return isSquareAttacked(pos, kx, ky, !whiteToMove);
}

// ---------- Move generation (pseudo-legal) ----------

std::vector<Move> generatePseudoMoves(const Position& pos, bool whiteToMove) {
    std::vector<Move> mv;

    auto addIfNotFriendly = [&](int fx,int fy,int tx,int ty, Flag fl = NONE, int promo=-1) {
        if (!inBounds(tx,ty)) return;
        if (friendlyAt(pos, whiteToMove, tx,ty)) return;
        if (enemyAt(pos, whiteToMove, tx,ty)) fl = (Flag)(fl | CAPTURE);
        mv.emplace_back(fx,fy,tx,ty,promo,fl);
    };

    // ----- Pawns -----
    {
        int dir = whiteToMove ? -1 : 1;                 // white moves up, black down
        int startRank = whiteToMove ? 6 : 1;            // initial rank for double push
        int promoRank = whiteToMove ? 0 : 7;            // target rank for promotion

        Board pb = pos[whiteToMove ? WHITE_PAWN : BLACK_PAWN];
        for (int x=0;x<8;x++) for (int y=0;y<8;y++) if (pb[x][y]) {
            int ny = y + dir;

            // Single push (empty square ahead)
            if (inBounds(x,ny) && !occupied(pos,x,ny)) {
                if (ny == promoRank) {
                    // promotions to Q,R,B,N
                    int pQ = whiteToMove ? WHITE_QUEEN : BLACK_QUEEN;
                    int pR = whiteToMove ? WHITE_ROOK  : BLACK_ROOK;
                    int pB = whiteToMove ? WHITE_BISHOP: BLACK_BISHOP;
                    int pN = whiteToMove ? WHITE_KNIGHT: BLACK_KNIGHT;
                    mv.emplace_back(x,y,x,ny,pQ, (Move::Flag)(Move::PROMOTION));
                    mv.emplace_back(x,y,x,ny,pR, (Move::Flag)(Move::PROMOTION));
                    mv.emplace_back(x,y,x,ny,pB, (Move::Flag)(Move::PROMOTION));
                    mv.emplace_back(x,y,x,ny,pN, (Move::Flag)(Move::PROMOTION));
                } else {
                    mv.emplace_back(x,y,x,ny,-1, Move::NONE);
                    // Double push from start if both squares empty
                    if (y == startRank) {
                        int ny2 = y + 2*dir;
                        if (inBounds(x,ny2) && !occupied(pos,x,ny2))
                            mv.emplace_back(x,y,x,ny2,-1, Move::DOUBLE_PAWN_PUSH);
                    }
                }
            }

            // Captures (x±1, y+dir)
            for (int dx : {-1,1}) {
                int nx = x + dx, nyc = y + dir;
                if (!inBounds(nx,nyc)) continue;

                // Regular capture
                if (enemyAt(pos, whiteToMove, nx,nyc)) {
                    if (nyc == promoRank) {
                        int pQ = whiteToMove ? WHITE_QUEEN : BLACK_QUEEN;
                        int pR = whiteToMove ? WHITE_ROOK  : BLACK_ROOK;
                        int pB = whiteToMove ? WHITE_BISHOP: BLACK_BISHOP;
                        int pN = whiteToMove ? WHITE_KNIGHT: BLACK_KNIGHT;
                        mv.emplace_back(x,y,nx,nyc,pQ, (Flag)(CAPTURE | PROMOTION));
                        mv.emplace_back(x,y,nx,nyc,pR, (Flag)(CAPTURE | PROMOTION));
                        mv.emplace_back(x,y,nx,nyc,pB, (Flag)(CAPTURE | PROMOTION));
                        mv.emplace_back(x,y,nx,nyc,pN, (Flag)(CAPTURE | PROMOTION));
                    } else {
                        mv.emplace_back(x,y,nx,nyc,-1, CAPTURE);
                    }
                }

                // En-passant capture: allowed only if pos.hasEnPassant() and target is that square
                if (pos.hasEnPassant()) {
                    if (pos.getEnPassantX() == nx && pos.getEnPassantY() == nyc) {
                        mv.emplace_back(x,y,nx,nyc,-1, (Move::Flag)(Move::EN_PASSANT | Move::CAPTURE));
                    }
                }
            }
        }
    }

    // ----- Knights -----
    {
        Board nb = pos[whiteToMove ? WHITE_KNIGHT : BLACK_KNIGHT];
        static const int K[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
        for (int x=0;x<8;x++) for (int y=0;y<8;y++) if (nb[x][y]) {
            for (auto& m : K) {
                int nx = x + m[0], ny = y + m[1];
                addIfNotFriendly(x,y,nx,ny);
            }
        }
    }

    // ----- Bishops -----
    {
        Board bb = pos[whiteToMove ? WHITE_BISHOP : BLACK_BISHOP];
        static const int D[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
        for (int x=0;x<8;x++) for (int y=0;y<8;y++) if (bb[x][y]) {
            for (auto& d : D) {
                int nx=x+d[0], ny=y+d[1];
                while (inBounds(nx,ny)) {
                    if (friendlyAt(pos,whiteToMove,nx,ny)) break;
                    Move::Flag fl = enemyAt(pos,whiteToMove,nx,ny) ? Move::CAPTURE : Move::NONE;
                    mv.emplace_back(x,y,nx,ny,-1,fl);
                    if (fl & Move::CAPTURE) break;
                    nx+=d[0]; ny+=d[1];
                }
            }
        }
    }

    // ----- Rooks -----
    {
        Board rb = pos[whiteToMove ? WHITE_ROOK : BLACK_ROOK];
        static const int D[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
        for (int x=0;x<8;x++) for (int y=0;y<8;y++) if (rb[x][y]) {
            for (auto& d : D) {
                int nx=x+d[0], ny=y+d[1];
                while (inBounds(nx,ny)) {
                    if (friendlyAt(pos,whiteToMove,nx,ny)) break;
                    Move::Flag fl = enemyAt(pos,whiteToMove,nx,ny) ? Move::CAPTURE : Move::NONE;
                    mv.emplace_back(x,y,nx,ny,-1,fl);
                    if (fl & Move::CAPTURE) break;
                    nx+=d[0]; ny+=d[1];
                }
            }
        }
    }

    // ----- Queens -----
    {
        Board qb = pos[whiteToMove ? WHITE_QUEEN : BLACK_QUEEN];
        static const int D[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
        for (int x=0;x<8;x++) for (int y=0;y<8;y++) if (qb[x][y]) {
            for (auto& d : D) {
                int nx=x+d[0], ny=y+d[1];
                while (inBounds(nx,ny)) {
                    if (friendlyAt(pos,whiteToMove,nx,ny)) break;
                    Move::Flag fl = enemyAt(pos,whiteToMove,nx,ny) ? Move::CAPTURE : Move::NONE;
                    mv.emplace_back(x,y,nx,ny,-1,fl);
                    if (fl & Move::CAPTURE) break;
                    nx+=d[0]; ny+=d[1];
                }
            }
        }
    }

    // ----- Kings -----
    {
        Board kb = pos[whiteToMove ? WHITE_KING : BLACK_KING];
        for (int x=0;x<8;x++) for (int y=0;y<8;y++) if (kb[x][y]) {
            for (int dx=-1; dx<=1; ++dx)
                for (int dy=-1; dy<=1; ++dy) {
                    if (!dx && !dy) continue;
                    int nx = x+dx, ny = y+dy;
                    addIfNotFriendly(x,y,nx,ny);
                }

            // Castling (pseudo-legal but respecting empty squares and attacked path)
            // NOTE: King may not be in check, and must not pass through an attacked square.
            // Coordinates are engine-dependent; here we assume standard:
            // White king starts at (4,7), rooks at (0,7) and (7,7).
            // Black king starts at (4,0), rooks at (0,0) and (7,0).
            bool sideWhite = whiteToMove;
            int ky = sideWhite ? 7 : 0;
            if (y == ky && x == 4) {
                // Short castle
                bool canShort = sideWhite ? pos.getShortWhiteCastling() : pos.getShortBlackCastling();
                if (canShort) {
                    // squares between king and rook empty: (5,ky) and (6,ky)
                    if (!occupied(pos,5,ky) && !occupied(pos,6,ky)) {
                        // none of (4,ky), (5,ky), (6,ky) attacked by opponent
                        if (!isSquareAttacked(pos,4,ky,!sideWhite) &&
                            !isSquareAttacked(pos,5,ky,!sideWhite) &&
                            !isSquareAttacked(pos,6,ky,!sideWhite)) {
                            mv.emplace_back(4,ky,6,ky,-1, Move::CASTLE_SHORT);
                        }
                    }
                }
                // Long castle
                bool canLong = sideWhite ? pos.getLongWhiteCastling() : pos.getLongBlackCastling();
                if (canLong) {
                    // squares between king and rook empty: (3,ky),(2,ky),(1,ky) must be empty
                    if (!occupied(pos,3,ky) && !occupied(pos,2,ky) && !occupied(pos,1,ky)) {
                        if (!isSquareAttacked(pos,4,ky,!sideWhite) &&
                            !isSquareAttacked(pos,3,ky,!sideWhite) &&
                            !isSquareAttacked(pos,2,ky,!sideWhite)) {
                            mv.emplace_back(4,ky,2,ky,-1, Move::CASTLE_LONG);
                        }
                    }
                }
            }
        }
    }

    return mv;
}

// ---------- Apply move (produces a new Position) ----------

Position applyMove(const Position& pos, const Move& mv, bool whiteToMove) {
    Position npos(pos);

    // Helper to clear a square from any piece
    auto clearSquare = [&](int x,int y) {
        for (int f=WHITE_PAWN; f<=BLACK_KING; ++f) npos[(Figures)f][x][y] = 0;
    };

    // Helper: move a single specific piece kind from (fx,fy) to (tx,ty)
    auto movePieceKind = [&](Figures f, int fx,int fy,int tx,int ty) {
        npos[f][fx][fy] = 0;
        npos[f][tx][ty] = 1;
    };

    // Identify moving side's piece kind at from-square
    Figures fs = whiteToMove ? WHITE_PAWN : BLACK_PAWN;
    Figures fe = whiteToMove ? WHITE_KING : BLACK_KING;
    Figures movingKind = WHITE_PAWN; bool found=false;
    for (int f=fs; f<=fe; ++f) {
        if (npos[(Figures)f][mv.fromX][mv.fromY]) { movingKind = (Figures)f; found=true; break; }
    }
    if (!found) return npos; // invalid input, do nothing

    // Default: reset en-passant
    npos.hasEnPassant() = false;
    npos.getEnPassantX() = -1;
    npos.getEnPassantY() = -1;

    // Handle captures on destination (regular capture)
    if (mv.flag & Move::CAPTURE) {
        // If this is EN_PASSANT, the captured pawn is not on 'to' but behind it
        if (mv.flag & Move::EN_PASSANT) {
            int capY = whiteToMove ? mv.toY + 1 : mv.toY - 1;
            Figures os = whiteToMove ? BLACK_PAWN : WHITE_PAWN;
            Figures oe = whiteToMove ? BLACK_KING : WHITE_KING;
            // specifically remove a pawn from ep capture square
            npos[(whiteToMove ? BLACK_PAWN : WHITE_PAWN)][mv.toX][capY] = 0;
        } else {
            // remove any opponent piece present on 'to'
            Figures os = whiteToMove ? BLACK_PAWN : WHITE_PAWN;
            Figures oe = whiteToMove ? BLACK_KING : WHITE_KING;
            for (int f=os; f<=oe; ++f) npos[(Figures)f][mv.toX][mv.toY] = 0;
        }
    }

    // Move execution paths
    if (mv.flag & Move::CASTLE_SHORT) {
        // Move king
        movePieceKind(whiteToMove ? WHITE_KING : BLACK_KING, mv.fromX,mv.fromY, mv.toX,mv.toY);
        // Move rook from h-file to f-file
        int ky = whiteToMove ? 7 : 0;
        movePieceKind(whiteToMove ? WHITE_ROOK : BLACK_ROOK, 7,ky, 5,ky);
        // Update castling rights
        if (whiteToMove) {
            npos.getShortWhiteCastling() = false;
            npos.getLongWhiteCastling()  = false;
            npos.isWhiteCastled() = true;
        } else {
            npos.getShortBlackCastling() = false;
            npos.getLongBlackCastling()  = false;
            npos.isBlackCastled() = true;
        }
    } else if (mv.flag & Move::CASTLE_LONG) {
        // Move king
        movePieceKind(whiteToMove ? WHITE_KING : BLACK_KING, mv.fromX,mv.fromY, mv.toX,mv.toY);
        // Move rook from a-file to d-file
        int ky = whiteToMove ? 7 : 0;
        movePieceKind(whiteToMove ? WHITE_ROOK : BLACK_ROOK, 0,ky, 
