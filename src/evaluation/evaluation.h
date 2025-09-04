#ifndef EVALUATION_H
#define EVALUATION_H

#include "../utils/position.h"

#define PENALTY_DOUBLE_PAWNS 25
#define PENALTY_ISOLATED_PAWNS 30
#define PENALTY_NOT_CASTLED_IF_POSSIBLE 10
#define PENALTY_NOT_CASTLED 5

namespace Evaluation {
    double evaluate(Position pos);
}

#endif // EVALUATION_H