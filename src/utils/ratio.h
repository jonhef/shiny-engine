#ifndef RATIO_H
#define RATIO_H

#include <string>

#include "position.h"
#include "board.h"

class Ratio {
    double coeffecients[8][8];
    Figures piece;
public:
    Ratio();
    Ratio(Figures piece);
    Ratio(double coeffecients[8][8], Figures piece);
    ~Ratio();

    double operator*(Position pos);
};

#endif // RATIO_H