#ifndef RATIO_H
#define RATIO_H

#include "position.h"
#include "figures.h"

class Ratio {
public:
    double coeffecients[8][8];
    Figures piece;

    Ratio();
    Ratio(double coeffecients[8][8], Figures piece);
    Ratio(Figures piece);
    ~Ratio();

    double operator*(const Position& pos);
};

#endif // RATIO_H
