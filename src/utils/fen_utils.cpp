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
        default:           return ' ';
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

std::string encodeFEN(const Position& pos) {
    std::string fen;
    for (int row = 0; row < 8; ++row) {
        int emptyCount = 0;
        for (int col = 0; col < 8; ++col) {
            int piece = getPieceAt(pos, row, col);
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
        if (emptyCount > 0) {
            fen += std::to_string(emptyCount);
        }
        if (row < 7) {
            fen += '/';
        }
    }
    return fen;
}

void decodeFEN(const std::string &fen, Position& board) {
    std::vector<std::string> rows;
    std::istringstream iss(fen);
    std::string rowStr;
    while (std::getline(iss, rowStr, '/') && rows.size() < 8) {
        rows.push_back(rowStr);
    }
    
    for (int row = 0; row < 8; ++row) {
        int col = 0;
        for (char c : rows[row]) {
            if (std::isdigit(c)) {
                int numEmpty = c - '0';
                for (int i = 0; i < numEmpty; ++i) {
                    setPieceAt(board, row, col, int(EMPTY));
                    col++;
                }
            } else {
                int piece = charToPiece(c);
                setPieceAt(board, row, col, piece);
                col++;
            }
        }
    }
}