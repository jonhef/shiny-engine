#ifndef BOARD_H
#define BOARD_H

class Row {
    bool row[8];
public:
    Row();
    Row(int row[8]);
    ~Row();

    bool& operator[](int index);
    bool operator[](int index) const;
};

class Board {
    Row board[8];
public:
    Board();
    Board(Board& board);
    Board(const Board& board);
    ~Board();

    Row& operator[](int index); 
    Row operator[](int index) const;
};

#endif // BOARD_H