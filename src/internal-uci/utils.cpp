#include <sstream>
#include <string>
#include <iostream>
#include <unordered_map>

#include "position/position.h"
#include "fen/fen.h"
#include "searching/pvs.h"
#include "searching/searching.h"
#include "uci.h"

constexpr int THREADS = 32;

// UCI → Move
Move parseUCIMove(const std::string& moveStr) {
    Move m{};
    m.fromX = moveStr[0] - 'a';  // file
    m.fromY = moveStr[1] - '1';  // rank
    m.toX   = moveStr[2] - 'a';
    m.toY   = moveStr[3] - '1';

    m.promotion = EMPTY;
    if (moveStr.size() == 5) {
        switch (moveStr[4]) {
            case 'q': m.promotion = QUEEN; break;
            case 'r': m.promotion = ROOK;  break;
            case 'b': m.promotion = BISHOP;break;
            case 'n': m.promotion = KNIGHT;break;
            default: break;
        }
    }
    return m;
}

// Move → UCI
std::string encodeUCIMove(const Move& mv) {
    std::string res;
    res += char('a' + mv.fromX);  // file
    res += char('1' + mv.fromY);  // rank
    res += char('a' + mv.toX);
    res += char('1' + mv.toY);
    if (mv.promotion != EMPTY) {
        switch (mv.promotion) {
            case QUEEN:  res += 'q'; break;
            case ROOK:   res += 'r'; break;
            case BISHOP: res += 'b'; break;
            case KNIGHT: res += 'n'; break;
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
        res = iterativeDeepeningThreadsDepth(pos, depth, tt, THREADS);
    } else if (movetime > 0) {
        res = iterativeDeepeningThreadsTime(pos, movetime, tt, THREADS);
    } else {
        // простая эвристика для лимита по времени
        int myTime = pos.isWhiteToMove() ? wtime : btime;
        int inc    = pos.isWhiteToMove() ? winc : binc;
        if (myTime <= 0) myTime = 1000; // fallback 1s
        int moves = movestogo > 0 ? movestogo : 30;
        int alloc = myTime / moves + inc;
        if (alloc < 50) alloc = 50;
        res = iterativeDeepeningThreadsTime(pos, alloc, tt, THREADS);
    }

    std::cout << "bestmove " << encodeUCIMove(res.bestMove) << std::endl;
}

void handleOpts(const std::string& line, std::unordered_map<std::string, std::string>& opts) {

}