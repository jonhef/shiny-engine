#include "pvs.h"

#include <random>

uint64_t zobristTable[2][6][8][8];
uint64_t zobristSide;
uint64_t zobristCastle[16];
uint64_t zobristEnPassant[8];

void initZobrist() {
    std::mt19937_64 rng(2025); // фиксированный сид для стабильности
    std::uniform_int_distribution<uint64_t> dist;

    for (int c=0;c<2;++c)
        for (int p=0;p<6;++p)
            for (int f=0;f<8;++f)
                for (int r=0;r<8;++r)
                    zobristTable[c][p][f][r] = dist(rng);

    zobristSide = dist(rng);
    for (int i=0;i<16;++i) zobristCastle[i] = dist(rng);
    for (int f=0;f<8;++f) zobristEnPassant[f] = dist(rng);
}

uint64_t computeHash(const Position& pos) {
    uint64_t h = 0;

    for (int f=0; f<8; ++f) {
        for (int r=0; r<8; ++r) {
            Piece p = pos.getPiece(f, r);
            if (p.getType() != EMPTY) {
                int color = p.isWhite() ? 0 : 1;
                int typeIndex;
                switch (p.getType()) {
                    case PAWN: typeIndex=0; break;
                    case KNIGHT: typeIndex=1; break;
                    case BISHOP: typeIndex=2; break;
                    case ROOK: typeIndex=3; break;
                    case QUEEN: typeIndex=4; break;
                    case KING: typeIndex=5; break;
                    default: continue;
                }
                h ^= zobristTable[color][typeIndex][f][r];
            }
        }
    }

    if (pos.isWhiteToMove()) h ^= zobristSide;
    h ^= zobristCastle[pos.getCastleRights() & 0xF];
    auto ep = pos.getEnPassant();
    if (ep.first >= 0 && ep.first < 8 && ep.second >= 0 && ep.second < 8) {
        h ^= zobristEnPassant[ep.first];
    }

    return h;
}
