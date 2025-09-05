#include "tt.h"

Zobrist::Zobrist() {
    std::mt19937_64 rng(20250905); // фиксированный сид, можно рандом
    std::uniform_int_distribution<uint64_t> dist;

    // Фигуры
    for (int piece = 0; piece < 12; ++piece)
        for (int square = 0; square < 64; ++square)
            zobristTable[piece][square] = dist(rng);

    // Ход
    zobristSide = dist(rng);

    // Рокировки
    for (int i = 0; i < 16; ++i)
        zobristCastling[i] = dist(rng);

    // En passant
    for (int f = 0; f < 8; ++f)
        zobristEnPassant[f] = dist(rng);
}

Zobrist::~Zobrist() {
    
}

uint64_t Zobrist::computeHash(const Position& pos) const {
    uint64_t h = 0;

    // Фигуры
    for (int piece = 0; piece < 12; ++piece) {
        Board b = pos[(Figures)piece];
        for (int x = 0; x < 8; ++x)
            for (int y = 0; y < 8; ++y)
                if (b[x][y]) {
                    int square = y * 8 + x;
                    h ^= zobristTable[piece][square];
                }
    }

    // Чей ход
    if (pos.isWhiteMove())
        h ^= zobristSide;

    // Рокировки (закодируй права как 4 бита)
    int castlingRights = 0;
    if (pos.getShortWhiteCastling()) castlingRights |= 1;
    if (pos.getLongWhiteCastling())  castlingRights |= 2;
    if (pos.getShortBlackCastling()) castlingRights |= 4;
    if (pos.getLongBlackCastling())  castlingRights |= 8;
    h ^= zobristCastling[castlingRights];

    // En passant
    auto ep = pos.getEnPassantSquare();
    if (ep.first != -1) {
        h ^= zobristEnPassant[ep.first]; // только файл
    }

    return h;
}