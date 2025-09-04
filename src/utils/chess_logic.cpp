// chess_logic.cpp
// Full move generation and check/mate detection.
// Comments are in English.

#include <vector>
#include <utility>
#include <algorithm>
#include "position.h"
#include "figures.h"

// Representation of a move. Promotion piece is optional (0 = none, else one of WHITE_QUEEN ... BLACK_KING enum values).
struct Move {
    int fromX, fromY;
    int toX, toY;
    Figures promotion; // use the promoted piece FIGURE (e.g. WHITE_QUEEN) or  (Figures) -1 for none

    Move(int fx=0,int fy=0,int tx=0,int ty=0, Figures p=(Figures)-1) :
        fromX(fx), fromY(fy), toX(tx), toY(ty), promotion(p) {}
};

// Utility: board bounds
inline bool inBounds(int x, int y) {
    return x >= 0 && x < 8 && y >= 0 && y < 8;
}

// Utility: iterate all piece types of a side
inline std::pair<int,int> sideRange(bool white) {
    if (white) return std::pair<int, int>(WHITE_PAWN, WHITE_KING);
    return std::pair<int, int>(BLACK_PAWN, BLACK_KING);
}

// Check if target square contains any friendly piece (for side whiteToMove)
auto isFriendlyAt = [&](const Position& pos, bool whiteToMove, int x, int y) -> bool {
    auto r = sideRange(whiteToMove);
    for (int f = r.first; f <= r.second; ++f) {
        if (pos[(Figures)f][x][y]) return true;
    }
    return false;
};

// Check if target square contains an enemy piece
auto isEnemyAt = [&](const Position& pos, bool whiteToMove, int x, int y) -> bool {
    auto r = sideRange(!whiteToMove);
    for (int f = r.first; f <= r.second; ++f) {
        if (pos[(Figures)f][x][y]) return true;
    }
    return false;
};

// Find the king position for given side
std::pair<int,int> findKing(const Position& pos, bool white) {
    Figures king = Figures(white ? WHITE_KING : BLACK_KING);
    Board b = pos[king];
    for (int x = 0; x < 8; ++x) for (int y = 0; y < 8; ++y)
        if (b[x][y]) return {x,y};
    return {-1,-1};
}

// Check whether square (x,y) is attacked by side `byWhite`.
bool isSquareAttacked(const Position& pos, int x, int y, bool byWhite) {
    // Pawns attack diagonally forward relative to their color.
    if (byWhite) {
        // white pawns move up (y-1), attack (x-1,y-1) and (x+1,y-1)
        if (inBounds(x-1,y-1) && pos[WHITE_PAWN][x-1][y-1]) return true;
        if (inBounds(x+1,y-1) && pos[WHITE_PAWN][x+1][y-1]) return true;
    } else {
        // black pawns move down (y+1), attack (x-1,y+1) and (x+1,y+1)
        if (inBounds(x-1,y+1) && pos[BLACK_PAWN][x-1][y+1]) return true;
        if (inBounds(x+1,y+1) && pos[BLACK_PAWN][x+1][y+1]) return true;
    }

    // Knights
    static const int knightMoves[8][2] = {
        {1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}
    };
    for (auto &m : knightMoves) {
        int nx = x + m[0], ny = y + m[1];
        if (!inBounds(nx,ny)) continue;
        if (byWhite) {
            if (pos[WHITE_KNIGHT][nx][ny]) return true;
        } else {
            if (pos[BLACK_KNIGHT][nx][ny]) return true;
        }
    }

    // Sliding pieces: bishops/queens (diagonals)
    static const int diagDirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
    for (auto &d : diagDirs) {
        int nx = x + d[0], ny = y + d[1];
        while (inBounds(nx,ny)) {
            // if a piece of attacking side appears and is bishop or queen => attacked
            if (byWhite) {
                if (pos[WHITE_BISHOP][nx][ny] || pos[WHITE_QUEEN][nx][ny]) return true;
                // if any piece blocks further sliding, stop
                bool any=false;
                for (int f = WHITE_PAWN; f <= WHITE_KING; ++f) if (pos[(Figures)f][nx][ny]) { any=true; break; }
                if (any) break;
            } else {
                if (pos[BLACK_BISHOP][nx][ny] || pos[BLACK_QUEEN][nx][ny]) return true;
                bool any=false;
                for (int f = BLACK_PAWN; f <= BLACK_KING; ++f) if (pos[(Figures)f][nx][ny]) { any=true; break; }
                if (any) break;
            }
            nx += d[0]; ny += d[1];
        }
    }

    // Rooks/queens (orthogonal)
    static const int orthoDirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
    for (auto &d : orthoDirs) {
        int nx = x + d[0], ny = y + d[1];
        while (inBounds(nx,ny)) {
            if (byWhite) {
                if (pos[WHITE_ROOK][nx][ny] || pos[WHITE_QUEEN][nx][ny]) return true;
                bool any=false;
                for (int f=WHITE_PAWN; f<=WHITE_KING; ++f) if (pos[(Figures)f][nx][ny]) { any=true; break; }
                if (any) break;
            } else {
                if (pos[BLACK_ROOK][nx][ny] || pos[BLACK_QUEEN][nx][ny]) return true;
                bool any=false;
                for (int f=BLACK_PAWN; f<=BLACK_KING; ++f) if (pos[(Figures)f][nx][ny]) { any=true; break; }
                if (any) break;
            }
            nx += d[0]; ny += d[1];
        }
    }

    // King (adjacent squares)
    for (int dx=-1; dx<=1; ++dx) for (int dy=-1; dy<=1; ++dy) {
        if (dx==0 && dy==0) continue;
        int nx = x + dx, ny = y + dy;
        if (!inBounds(nx,ny)) continue;
        if (byWhite) {
            if (pos[WHITE_KING][nx][ny]) return true;
        } else {
            if (pos[BLACK_KING][nx][ny]) return true;
        }
    }

    return false;
}

// Check if side `whiteToMove` is in check
bool isCheck(const Position& pos, bool whiteToMove) {
    auto kingPos = findKing(pos, whiteToMove);
    if (kingPos.first == -1) return false; // invalid position (no king) -> not reporting check
    // attacked by opponent side
    return isSquareAttacked(pos, kingPos.first, kingPos.second, !whiteToMove);
}

// Apply a move to a copy of position. Handles captures, promotion, en-passant, castling.
Position applyMove(const Position& pos, const Move& mv, bool whiteToMove) {
    Position newPos(pos);

    // 1. find which piece moves (scan the side's pieces)
    auto pr = sideRange(whiteToMove);
    Figures movingPiece = (Figures)-1;
    for (int f = pr.first; f <= pr.second; ++f) {
        if (newPos[(Figures)f][mv.fromX][mv.fromY]) { movingPiece = (Figures)f; break; }
    }

    if (movingPiece == (Figures)-1) {
        // nothing to move: return original copy
        return newPos;
    }

    // Clear source
    newPos[movingPiece][mv.fromX][mv.fromY] = 0;

    // En-passant handling: if pawn moves diagonally to empty square equal to enPassantSquare, capture pawn behind
    if ((movingPiece == WHITE_PAWN || movingPiece == BLACK_PAWN) && newPos[(Figures)0][0][0]==0) {
        // note: the above dummy check is just to silence compiler if needed; we'll handle properly below
    }
    // Normal capture: clear any opponent piece on target square
    auto oppRange = sideRange(!whiteToMove);
    for (int f = oppRange.first; f <= oppRange.second; ++f) {
        newPos[(Figures)f][mv.toX][mv.toY] = 0;
    }

    // Handle en-passant capture:
    auto ep = newPos.getEnPassantSquare();
    if ((movingPiece == WHITE_PAWN || movingPiece == BLACK_PAWN) && ep.first != -1) {
        // if pawn moves to en-passant target square (diagonal) and target square currently empty, then capture the pawn
        if (mv.toX == ep.first && mv.toY == ep.second) {
            // captured pawn is on the previous rank of the mover
            if (whiteToMove) {
                // white captures black pawn that was on (ep.first, ep.second+1)
                int capY = ep.second + 1;
                if (inBounds(ep.first, capY)) {
                    newPos[BLACK_PAWN][ep.first][capY] = 0;
                }
            } else {
                // black captures white pawn that was on (ep.first, ep.second-1)
                int capY = ep.second - 1;
                if (inBounds(ep.first, capY)) {
                    newPos[WHITE_PAWN][ep.first][capY] = 0;
                }
            }
        }
    }

    // Put moving piece (or promotion)
    if (mv.promotion != (Figures)-1) {
        // promotion: put promoted piece of correct color at target
        newPos[mv.promotion][mv.toX][mv.toY] = 1;
        // ensure pawn is removed (we already cleared source)
    } else {
        newPos[movingPiece][mv.toX][mv.toY] = 1;
    }

    // Castling: if king moved two squares horizontally, move rook accordingly and update castling flags
    if (movingPiece == WHITE_KING) {
        // disable white castling rights
        newPos.getLongWhiteCastling() = false;
        newPos.getShortWhiteCastling() = false;
        // detect king-side castling (e1->g1 : x from 4->6)
        if (mv.fromX == 4 && mv.fromY == 7 && mv.toX == 6 && mv.toY == 7) {
            // move rook from h1 (7,7) to f1 (5,7)
            newPos[WHITE_ROOK][7][7] = 0;
            newPos[WHITE_ROOK][5][7] = 1;
            newPos.isWhiteCastled() = true;
        }
        // queen-side castling e1->c1 (4->2)
        if (mv.fromX == 4 && mv.fromY == 7 && mv.toX == 2 && mv.toY == 7) {
            newPos[WHITE_ROOK][0][7] = 0;
            newPos[WHITE_ROOK][3][7] = 1;
            newPos.isWhiteCastled() = true;
        }
    } else if (movingPiece == BLACK_KING) {
        newPos.getLongBlackCastling() = false;
        newPos.getShortBlackCastling() = false;
        if (mv.fromX == 4 && mv.fromY == 0 && mv.toX == 6 && mv.toY == 0) {
            newPos[BLACK_ROOK][7][0] = 0;
            newPos[BLACK_ROOK][5][0] = 1;
            newPos.isBlackCastled() = true;
        }
        if (mv.fromX == 4 && mv.fromY == 0 && mv.toX == 2 && mv.toY == 0) {
            newPos[BLACK_ROOK][0][0] = 0;
            newPos[BLACK_ROOK][3][0] = 1;
            newPos.isBlackCastled() = true;
        }
    }

    // If rook moved, disable corresponding castling rights
    if (movingPiece == WHITE_ROOK) {
        if (mv.fromX == 0 && mv.fromY == 7) newPos.getLongWhiteCastling() = false;
        if (mv.fromX == 7 && mv.fromY == 7) newPos.getShortWhiteCastling() = false;
    }
    if (movingPiece == BLACK_ROOK) {
        if (mv.fromX == 0 && mv.fromY == 0) newPos.getLongBlackCastling() = false;
        if (mv.fromX == 7 && mv.fromY == 0) newPos.getShortBlackCastling() = false;
    }

    // If any capture of opponent rook on original rook squares, update opponent castling flags
    // (capture at a1/h1 or a8/h8)
    if (mv.toX == 0 && mv.toY == 7) newPos.getLongWhiteCastling() = false;
    if (mv.toX == 7 && mv.toY == 7) newPos.getShortWhiteCastling() = false;
    if (mv.toX == 0 && mv.toY == 0) newPos.getLongBlackCastling() = false;
    if (mv.toX == 7 && mv.toY == 0) newPos.getShortBlackCastling() = false;

    // Update en-passant square:
    // If a pawn moved two squares, set en-passant target to the square behind it; otherwise clear.
    newPos.setEnPassantSquare({-1,-1});
    if (movingPiece == WHITE_PAWN) {
        if (mv.fromY == 6 && mv.toY == 4 && mv.fromX == mv.toX) {
            // black can capture on (mv.toX,5) en-passant
            newPos.setEnPassantSquare({mv.toX, 5});
        }
    } else if (movingPiece == BLACK_PAWN) {
        if (mv.fromY == 1 && mv.toY == 3 && mv.fromX == mv.toX) {
            // white can capture on (mv.toX,2) en-passant
            newPos.setEnPassantSquare({mv.toX, 2});
        }
    }

    // Toggle side to move
    newPos.isWhiteMove() = !whiteToMove;

    return newPos;
}

// Generate all pseudo-legal moves (ignoring whether king is left in check).
// This includes pawn promotions, en-passant targets (using pos.getEnPassantSquare()), and castling moves if rights exist.
// Note: castling will not be validated against passing through/into check here — validation happens later.
std::vector<Move> generatePseudoMoves(const Position& pos, bool whiteToMove) {
    std::vector<Move> moves;
    auto r = sideRange(whiteToMove);

    // helper lambdas
    auto friendlyAt = [&](int x,int y)->bool {
        for (int f = r.first; f<=r.second; ++f) if (pos[(Figures)f][x][y]) return true;
        return false;
    };
    auto enemyAt = [&](int x,int y)->bool {
        auto rr = sideRange(!whiteToMove);
        for (int f = rr.first; f<=rr.second; ++f) if (pos[(Figures)f][x][y]) return true;
        return false;
    };

    auto ep = pos.getEnPassantSquare();

    for (int f = r.first; f <= r.second; ++f) {
        Figures fig = (Figures)f;
        Board b = pos[fig];
        for (int x=0;x<8;++x) for (int y=0;y<8;++y) {
            if (!b[x][y]) continue;

            // PAWNS
            if (fig == WHITE_PAWN || fig == BLACK_PAWN) {
                int dir = (fig == WHITE_PAWN) ? -1 : +1;
                int startRank = (fig == WHITE_PAWN) ? 6 : 1;
                int promoteRank = (fig == WHITE_PAWN) ? 0 : 7;

                // single push
                int nx = x, ny = y + dir;
                if (inBounds(nx,ny) && !friendlyAt(nx,ny) && !enemyAt(nx,ny)) {
                    if (ny == promoteRank) {
                        // promotions: typically promote to Q,R,B,N
                        std::vector<Figures> promos;
                        if (fig == WHITE_PAWN) {
                            promos = {WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT};
                        } else {
                            promos = {BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT};
                        }
                        for (Figures p : promos) moves.emplace_back(x,y,nx,ny,p);
                    } else {
                        moves.emplace_back(x,y,nx,ny,(Figures)-1);
                        // double push from start
                        if (y == startRank) {
                            int ny2 = y + 2*dir;
                            if (inBounds(nx,ny2) && !friendlyAt(nx,ny2) && !enemyAt(nx,ny2)) {
                                moves.emplace_back(x,y,nx,ny2,(Figures)-1);
                            }
                        }
                    }
                }

                // captures
                for (int dx : {-1, +1}) {
                    int cx = x + dx, cy = y + dir;
                    if (!inBounds(cx,cy)) continue;
                    if (enemyAt(cx,cy)) {
                        if (cy == promoteRank) {
                            std::vector<Figures> promos;
                            if (fig == WHITE_PAWN) promos = {WHITE_QUEEN, WHITE_ROOK, WHITE_BISHOP, WHITE_KNIGHT};
                            else promos = {BLACK_QUEEN, BLACK_ROOK, BLACK_BISHOP, BLACK_KNIGHT};
                            for (Figures p : promos) moves.emplace_back(x,y,cx,cy,p);
                        } else {
                            moves.emplace_back(x,y,cx,cy,(Figures)-1);
                        }
                    }
                    // en-passant capture: target square equals ep and capture happens to empty square
                    if (ep.first != -1 && cx == ep.first && cy == ep.second) {
                        // ensure the captured pawn is directly behind the ep square
                        moves.emplace_back(x,y,cx,cy,(Figures)-1);
                    }
                }
            }

            // KNIGHTS
            if (fig == WHITE_KNIGHT || fig == BLACK_KNIGHT) {
                static const int kM[8][2] = {{1,2},{2,1},{-1,2},{-2,1},{1,-2},{2,-1},{-1,-2},{-2,-1}};
                for (auto &m : kM) {
                    int nx = x + m[0], ny = y + m[1];
                    if (!inBounds(nx,ny)) continue;
                    if (!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);
                }
            }

            // BISHOPS
            if (fig == WHITE_BISHOP || fig == BLACK_BISHOP) {
                static const int dirs[4][2] = {{1,1},{1,-1},{-1,1},{-1,-1}};
                for (auto &d : dirs) {
                    int nx = x + d[0], ny = y + d[1];
                    while (inBounds(nx,ny)) {
                        if (!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);
                        if (friendlyAt(nx,ny) || enemyAt(nx,ny)) break;
                        nx += d[0]; ny += d[1];
                    }
                }
            }

            // ROOKS
            if (fig == WHITE_ROOK || fig == BLACK_ROOK) {
                static const int dirs[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                for (auto &d : dirs) {
                    int nx = x + d[0], ny = y + d[1];
                    while (inBounds(nx,ny)) {
                        if (!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);
                        if (friendlyAt(nx,ny) || enemyAt(nx,ny)) break;
                        nx += d[0]; ny += d[1];
                    }
                }
            }

            // QUEENS
            if (fig == WHITE_QUEEN || fig == BLACK_QUEEN) {
                static const int dirs[8][2] = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1}};
                for (auto &d : dirs) {
                    int nx = x + d[0], ny = y + d[1];
                    while (inBounds(nx,ny)) {
                        if (!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);
                        if (friendlyAt(nx,ny) || enemyAt(nx,ny)) break;
                        nx += d[0]; ny += d[1];
                    }
                }
            }

            // KINGS (including castling)
            if (fig == WHITE_KING || fig == BLACK_KING) {
                for (int dx=-1; dx<=1; ++dx) for (int dy=-1; dy<=1; ++dy) {
                    if (dx==0 && dy==0) continue;
                    int nx = x + dx, ny = y + dy;
                    if (!inBounds(nx,ny)) continue;
                    if (!friendlyAt(nx,ny)) moves.emplace_back(x,y,nx,ny,(Figures)-1);
                }

                // Castling: add castling moves if rights exist and squares empty. Validation of not passing/ending in check will be done later.
                if (fig == WHITE_KING && x == 4 && y == 7) {
                    // king-side
                    if (pos.getShortWhiteCastling()) {
                        // squares f1 (5,7) and g1 (6,7) must be empty
                        if (!friendlyAt(5,7) && !enemyAt(5,7) && !friendlyAt(6,7) && !enemyAt(6,7)) {
                            moves.emplace_back(4,7,6,7,(Figures)-1);
                        }
                    }
                    // queen-side
                    if (pos.getLongWhiteCastling()) {
                        // squares b1(1,7), c1(2,7), d1(3,7)
                        if (!friendlyAt(1,7) && !enemyAt(1,7) && !friendlyAt(2,7) && !enemyAt(2,7) && !friendlyAt(3,7) && !enemyAt(3,7)) {
                            moves.emplace_back(4,7,2,7,(Figures)-1);
                        }
                    }
                }
                if (fig == BLACK_KING && x == 4 && y == 0) {
                    if (pos.getShortBlackCastling()) {
                        if (!friendlyAt(5,0) && !enemyAt(5,0) && !friendlyAt(6,0) && !enemyAt(6,0)) {
                            moves.emplace_back(4,0,6,0,(Figures)-1);
                        }
                    }
                    if (pos.getLongBlackCastling()) {
                        if (!friendlyAt(1,0) && !enemyAt(1,0) && !friendlyAt(2,0) && !enemyAt(2,0) && !friendlyAt(3,0) && !enemyAt(3,0)) {
                            moves.emplace_back(4,0,2,0,(Figures)-1);
                        }
                    }
                }
            }
        }
    }

    return moves;
}

// Validate pseudo-legal moves to produce legal moves (king not left in check and castling not through/into check)
std::vector<Move> getLegalMoves(const Position& pos, bool whiteToMove) {
    std::vector<Move> legal;
    auto pseudo = generatePseudoMoves(pos, whiteToMove);

    for (auto &mv : pseudo) {
        // Special-case: when move is castling, ensure the king does not pass through / land on check.
        bool isCastling = false;
        if ( (whiteToMove && mv.fromX==4 && mv.fromY==7 && (mv.toX==6 || mv.toX==2)) ||
             (!whiteToMove && mv.fromX==4 && mv.fromY==0 && (mv.toX==6 || mv.toX==2)) ) {
            isCastling = true;
        }

        // Apply move
        Position after = applyMove(pos, mv, whiteToMove);

        // If king is in check after move -> illegal
        if (isCheck(after, whiteToMove)) continue;

        // For castling: also ensure squares king passes through aren't attacked
        if (isCastling) {
            if (whiteToMove) {
                if (mv.toX == 6) { // king-side: check e1(4,7), f1(5,7), g1(6,7) not under attack by black
                    if (isSquareAttacked(pos,4,7,false) || isSquareAttacked(pos,5,7,false) || isSquareAttacked(pos,6,7,false)) continue;
                } else { // queen-side: e1,d1,c1
                    if (isSquareAttacked(pos,4,7,false) || isSquareAttacked(pos,3,7,false) || isSquareAttacked(pos,2,7,false)) continue;
                }
            } else {
                if (mv.toX == 6) { // black king-side
                    if (isSquareAttacked(pos,4,0,true) || isSquareAttacked(pos,5,0,true) || isSquareAttacked(pos,6,0,true)) continue;
                } else {
                    if (isSquareAttacked(pos,4,0,true) || isSquareAttacked(pos,3,0,true) || isSquareAttacked(pos,2,0,true)) continue;
                }
            }
        }

        // En-passant special legality: capturing en-passant can expose king to check from behind.
        // We must ensure that after the en-passant capture the king is not in check (already covered by isCheck(after,...)),
        // but because applyMove already removes the captured pawn, we are safe.

        // If all OK, move is legal
        legal.push_back(mv);
    }

    return legal;
}

bool isCheckmate(const Position& pos, bool whiteToMove) {
    if (!isCheck(pos, whiteToMove)) return false;
    auto legal = getLegalMoves(pos, whiteToMove);
    return legal.empty();
}

// Optionally, you can expose these functions via a header or use them from your engine directly.
// Example usage:
// Position p; bool white = p.isWhiteMove();
// bool in_check = isCheck(p, white);
// auto moves = getLegalMoves(p, white);
// bool mate = isCheckmate(p, white);

