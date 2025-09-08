
#ifndef POSITION_H
#define POSITION_H

#include <utility>
#include <array>

enum Figures {
    PAWN = 100,
    KNIGHT = 320,
    BISHOP = 330,
    ROOK = 500,
    QUEEN = 900,
    KING = 400,
    EMPTY = 0
};

class Piece {
    Figures type;
    // true = white, false = black
    bool color;
    // unneccessary params
    int x, y;
public: 
    Piece();
    /* bool color = true if white and black otherwise */
    Piece(Figures figure, bool color);
    /* x = 0, y = 0 is a1 */
    Piece(Figures figure, bool color, int x, int y);
    ~Piece();
    
    int isWhite() const;
    /* x = 0, y = 0 is a1 */
    std::pair<int, int> getPos() const;
    /* 
       Pawn = 100 
       Knight = 320
       Bishop = 330
       Rook = 500
       Queen = 900
       King = 400
    */
    Figures getType() const;

    // true for white
    void setColor(bool color);
    /* x = 0, y = 0 is a1 */
    void setPos(const std::pair<int, int>& pos);
    /* x = 0, y = 0 is a1 */
    void setPos(int x, int y);
    /* 
       Pawn = 100 
       Knight = 320
       Bishop = 330
       Rook = 500
       Queen = 900
       King = 400
    */
    void setType(Figures figure);
};

class Position {
    std::array<std::array<Piece, 8>, 8> board;

    bool isWhiteMove;
    std::pair<int, int> squareEnPassant;
    /* 
       0b0001 short white 
       0b0010 long white 
       0b0100 short black
       0b1000 long black
    */
    short castleRights;
public:
    Position();
    Position(
        std::array<std::array<Piece, 8>, 8>      board,
        bool                               isWhiteMove,
        bool                          shortWhiteCastle,
        bool                           longWhiteCastle,
        bool                          shortBlackCastle,
        bool                           longBlackCastle,
        std::pair<int, int> squareEnPassant = {-1, -1}
    );
    ~Position();
    
    Piece getPiece(int x, int y) const;
    
    /* it changes original piece's position 
       x = 0, y = 0 is a1 */
    void setPiece(int x, int y, Piece& piece);
    /* x = 0, y = 0 is a1 */
    void setPiece(int x, int y, Figures figure, bool color);

    bool isSquareAttacked(const std::pair<int, int>& pos) const;
    bool isCheck() const;
};

#endif