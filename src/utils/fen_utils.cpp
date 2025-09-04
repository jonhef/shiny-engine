#include "fen_utils.h"

#include <string>
#include <cctype>
#include <sstream>
#include <vector>

#include "figures.h"

char pieceToChar(int piece) {
    switch (piece) {
        case WHITE_PAWN:   return 'P';
        case WHITE_KNIGHT: return 'N';
        case WHITE_BISHOP: return 'B';
        case WHITE_ROOK:   return 'R';
        case WHITE_QUEEN:  return 'Q';
        case WHITE_KING:   return 'K';
        case BLACK_PAWN:   return 'p';
        case BLACK_KNIGHT: return 'n';
        case BLACK_BISHOP: return 'b';
        case BLACK_ROOK:   return 'r';
        case BLACK_QUEEN:  return 'q';
        case BLACK_KING:   return 'k';
        default:           return '1';
    }
}

int charToPiece(char c) {
    switch (c) {
        case 'P': return WHITE_PAWN;
        case 'N': return WHITE_KNIGHT;
        case 'B': return WHITE_BISHOP;
        case 'R': return WHITE_ROOK;
        case 'Q': return WHITE_QUEEN;
        case 'K': return WHITE_KING;
        case 'p': return BLACK_PAWN;
        case 'n': return BLACK_KNIGHT;
        case 'b': return BLACK_BISHOP;
        case 'r': return BLACK_ROOK;
        case 'q': return BLACK_QUEEN;
        case 'k': return BLACK_KING;
        default:  return EMPTY;
    }
}

// Helper function to get piece at a specific square
int getPieceAt(const Position& pos, int row, int col) {
    if (pos[WHITE_PAWN][row][col]) return WHITE_PAWN;
    if (pos[WHITE_KNIGHT][row][col]) return WHITE_KNIGHT;
    if (pos[WHITE_BISHOP][row][col]) return WHITE_BISHOP;
    if (pos[WHITE_ROOK][row][col]) return WHITE_ROOK;
    if (pos[WHITE_QUEEN][row][col]) return WHITE_QUEEN;
    if (pos[WHITE_KING][row][col]) return WHITE_KING;
    if (pos[BLACK_PAWN][row][col]) return BLACK_PAWN;
    if (pos[BLACK_KNIGHT][row][col]) return BLACK_KNIGHT;
    if (pos[BLACK_BISHOP][row][col]) return BLACK_BISHOP;
    if (pos[BLACK_ROOK][row][col]) return BLACK_ROOK;
    if (pos[BLACK_QUEEN][row][col]) return BLACK_QUEEN;
    if (pos[BLACK_KING][row][col]) return BLACK_KING;
    return EMPTY;
}

// Helper function to set a piece at a specific square
void setPieceAt(Position& pos, int row, int col, int piece) {
    // Clear all pieces at this square
    pos[WHITE_PAWN][row][col] = false;
    pos[WHITE_KNIGHT][row][col] = false;
    pos[WHITE_BISHOP][row][col] = false;
    pos[WHITE_ROOK][row][col] = false;
    pos[WHITE_QUEEN][row][col] = false;
    pos[WHITE_KING][row][col] = false;
    pos[BLACK_PAWN][row][col] = false;
    pos[BLACK_KNIGHT][row][col] = false;
    pos[BLACK_BISHOP][row][col] = false;
    pos[BLACK_ROOK][row][col] = false;
    pos[BLACK_QUEEN][row][col] = false;
    pos[BLACK_KING][row][col] = false;

    // Set the new piece
    if (piece != EMPTY) {
        pos[piece][row][col] = true;
    }
}

void decodeFEN(const std::string &fen, Position& board) {
    std::istringstream iss(fen);
    std::string rowStr;
    int fenRow = 0;

    // Считаем строки до пробела (только доска)
    while (std::getline(iss, rowStr, ' ') && fenRow < 1) {
        // берем первую часть FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
        std::string boardPart = rowStr;
        std::vector<std::string> rows;
        std::istringstream rowStream(boardPart);
        std::string s;
        while (std::getline(rowStream, s, '/')) {
            rows.push_back(s);
        }

        for (int r = 0; r < 8; ++r) {
            const std::string &line = rows[r];
            int row = 7 - r;  // FEN верхняя строка = row 7
            int col = 0;
            for (char c : line) {
                if (std::isdigit(c)) {
                    int emptyCount = c - '0';
                    for (int i = 0; i < emptyCount; ++i) {
                        setPieceAt(board, col, row, EMPTY);
                        col++;
                    }
                } else {
                    int piece = charToPiece(c);
                    setPieceAt(board, col, row, piece);
                    col++;
                }
            }
        }
        fenRow++;
    }
}

std::string encodeFEN(const Position& pos) {
    std::string fen;
    for (int r = 7; r >= 0; --r) {  // сверху вниз, чтобы FEN был правильным
        int emptyCount = 0;
        for (int c = 0; c < 8; ++c) {
            int piece = getPieceAt(pos, c, r);
            if (piece == EMPTY) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen += std::to_string(emptyCount);
                    emptyCount = 0;
                }
                fen += pieceToChar(piece);
            }
        }
        if (emptyCount > 0) fen += std::to_string(emptyCount);
        if (r > 0) fen += '/';
    }
    return fen;
}