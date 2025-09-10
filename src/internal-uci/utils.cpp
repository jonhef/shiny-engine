#include <sstream>
#include <string>
#include <iostream>

#include "position/position.h"
#include "fen/fen.h"
#include "searching/pvs.h"
#include "searching/searching.h"
#include "uci.h"

// convert into move
Move parseUCIMove(const std::string& moveStr) {
    Move m{};
    // буквы файлов → 0..7
    m.fromY = moveStr[0] - 'a';
    m.fromX = moveStr[1] - '1';
    m.toY   = moveStr[2] - 'a';
    m.toX   = moveStr[3] - '1';

    m.promotion = EMPTY;
    if (moveStr.size() == 5) {
        char promo = moveStr[4];
        switch (promo) {
            case 'q': m.promotion = QUEEN; break;
            case 'r': m.promotion = ROOK; break;
            case 'b': m.promotion = BISHOP; break;
            case 'n': m.promotion = KNIGHT; break;
            default: m.promotion = EMPTY; break;
        }
    }

    return m;
}

std::string encodeUCIMove(const Move& mv) {
    std::string res = "";
    res += mv.fromY + 'a';
    res += mv.fromX + '1';
    res +=   mv.toY + 'a';
    res +=   mv.toX + '1';
    if (mv.promotion != EMPTY) {
        switch (mv.promotion) {
            case QUEEN: res += "q"; break;
            case ROOK: res += "r"; break;
            case BISHOP: res += "b"; break;
            case KNIGHT: res += "n"; break;
            default: break;
        }
    }
    return res;
}

// Обработка UCI-команды "position ..."
void handlePosition(const std::string& line, Position& pos) {
    std::istringstream iss(line);
    std::string token;
    iss >> token; // "position"
    iss >> token;

    if (token == "startpos") {
        decodeFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", pos);

        // check for moves
        if (iss >> token && token == "moves") {
            std::string moveStr;
            while (iss >> moveStr) {
                Move m = parseUCIMove(moveStr);
                pos.applyMove(m);
            }
        }
    }
    else if (token == "fen") {
        // make a fen from parts
        std::string fen, part;
        for (int i = 0; i < 6 && iss >> part; i++) {
            fen += part + " ";
        }

        decodeFEN(fen, pos);

        if (iss >> token && token == "moves") {
            std::string moveStr;
            while (iss >> moveStr) {
                Move m = parseUCIMove(moveStr);
                pos.applyMove(m);
            }
        }
    }
}

void handleGo(const std::string& line, Position& pos, TranspositionTable& tt) {
    std::istringstream iss(line);
    std::string token;
    iss >> token; // "go"

    int wtime = -1, btime = -1, winc = 0, binc = 0;
    int movetime = -1, movestogo = 0, depth = -1;
    bool infinite = false;

    while (iss >> token) {
        if (token == "wtime") iss >> wtime;
        else if (token == "btime") iss >> btime;
        else if (token == "winc")  iss >> winc;
        else if (token == "binc")  iss >> binc;
        else if (token == "movestogo") iss >> movestogo;
        else if (token == "movetime") iss >> movetime;
        else if (token == "depth") iss >> depth;
        else if (token == "nodes") { int nodes; iss >> nodes; /* пока игнор */ }
        else if (token == "infinite") infinite = true;
    }

    SearchResult res;

    if (depth > 0) {
        // ограничение по глубине
        res = iterativeDeepeningDepth(pos, depth, tt);
    } else if (movetime > 0) {
        res = iterativeDeepeningTime(pos, movetime, tt);
    } else {
        // простая эвристика для лимита по времени
        int myTime = pos.isWhiteToMove() ? wtime : btime;
        int inc    = pos.isWhiteToMove() ? winc : binc;
        if (myTime <= 0) myTime = 1000; // fallback 1s
        int moves = movestogo > 0 ? movestogo : 30;
        int alloc = myTime / moves + inc;
        if (alloc < 50) alloc = 50;
        res = iterativeDeepeningTime(pos, alloc, tt);
    }

    std::cout << "bestmove " << encodeUCIMove(res.bestMove) << std::endl;
}
