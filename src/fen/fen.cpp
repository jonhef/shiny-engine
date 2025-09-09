#include "fen.h"
#include <string>
#include <sstream>

bool decodeFEN(const std::string& fen, Position& pos) {
    std::stringstream ss(fen);
    std::string row;

    for (int i = 7; i >= 0; --i) {
        std::getline(ss, row, '/');
        auto iter = row.begin();
        for (int j = 0; j < 8; ++j) {
            switch (*iter) {
                case 'p':
                    pos.setPiece(i, j, PAWN, BLACK);
                    break;
                case 'P':
                    pos.setPiece(i, j, PAWN, WHITE);
                case 'r':
                    pos.setPiece(i, j, ROOK, BLACK);
                    break;
                case 'R':
                    pos.setPiece(i, j, ROOK, WHITE);
                    break;
                case 'n':
                    pos.setPiece(i, j, KNIGHT, BLACK);
                    break;
                case 'N':
                    pos.setPiece(i, j, KNIGHT, WHITE);
                    break;
                case 'b':
                    pos.setPiece(i, j, BISHOP, BLACK);
                    break;
                case 'B':
                    pos.setPiece(i, j, BISHOP, WHITE);
                    break;
                case 'q':
                    pos.setPiece(i, j, QUEEN, BLACK);
                    break;
                case 'Q':
                    pos.setPiece(i, j, QUEEN, WHITE);
                    break;
                case 'k':
                    pos.setPiece(i, j, KING, BLACK);
                    break;
                case 'K':
                    pos.setPiece(i, j, KING, WHITE);
                    break;
            }
            if (*iter >= '0' && '9' >= *iter) {
                for (int y = j; y < j + int(*iter - '0'); ++y) {
                    pos.setPiece(i, j, WHITE, false);
                }
                j += int(*iter - '0');
            }
            ++iter;
        }
    }
    
    // side to move
    std::getline(ss, row, ' ');
    if (row == "w") {
        pos.setIsWhiteMove(WHITE);
    } else {
        pos.setIsWhiteMove(BLACK);
    }

    // castling rights
    std::getline(ss, row, ' ');
    if (row != "-") {
        short castleRights = 0;
        auto iter = row.begin();
        while (iter != row.end()) {
            switch (*iter) {
            case 'K':
            castleRights |= 0b0001;
            case 'Q':
            castleRights |= 0b0010;
            case 'k':
            castleRights |= 0b0100;
            case 'q':
            castleRights |= 0b1000;
            }
            ++iter;
        }
        pos.setCastleRights(castleRights);
    }

    // en passant target square
    std::getline(ss, row, ' ');
    if (row != "-") {
        pos.setEnPassant(int(row[0] - 'a'), int(row[1] - '0'));
    }

    return true;
}

bool encodeFEN(std::string& fen, const Position& pos) {
    fen.clear();

    for (int i = 7; i >= 0; --i) {
        int mpties = 0;
        for (int j = 0; j < 8; ++j) {
            Piece type = pos.getPiece(i, j);
            if (type.getType() == EMPTY) {
                ++mpties;
                continue;
            } else if (mpties != 0) {
                fen.append(std::to_string(mpties));
            }
            mpties = 0;
            switch (type.getType()) {
            case PAWN:
                fen.append(type.isWhite() ? "P" : "p");
                break;
            case KNIGHT: 
                fen.append(type.isWhite() ? "N" : "n");
                break;
            case BISHOP:
                fen.append(type.isWhite() ? "B" : "b");
                break;
            case ROOK:
                fen.append(type.isWhite() ? "R" : "r");
                break;
            case QUEEN: 
                fen.append(type.isWhite() ? "Q" : "q");
                break;
            case KING: 
                fen.append(type.isWhite() ? "K" : "k");
                break;
            default: break;
            }
        }
        fen.append("/");
    }
    fen.pop_back();
    
    // side to move
    fen.append(" ");
    if (pos.isWhiteToMove()) {
        fen.append("w");
    } else {
        fen.append("b");
    }

    // castling rights
    fen.append(" ");
    short castlingRights = pos.getCastleRights();
    if (castlingRights & 0b0001) {
        fen.append("K");
    }
    if (castlingRights & 0b0010) {
        fen.append("Q");
    }
    if (castlingRights & 0b0100) {
        fen.append("k");
    }
    if (castlingRights & 0b1000) {
        fen.append("q");
    }

    // en passant square
    fen.append(" ");
    if (pos.getEnPassant().first != -1) {
        fen.append(std::string(
            char((int)'a' + pos.getEnPassant().first), 
            char((int)'0' + pos.getEnPassant().second)
        ));
    }

    // add for halfmove clock and all moves
    fen.append(" 0 1");
}