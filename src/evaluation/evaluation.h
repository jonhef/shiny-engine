#ifndef EVALUATION_H
#define EVALUATION_H

#include "../utils/position.h"

#define PENALTY_DOUBLE_PAWNS 25

namespace Evaluation {
    double evaluate(Position pos);
}

#endif // EVALUATION_H