#include "pvs.h"
#include <limits>
#include <chrono>
#include "searching.h"

SearchResult iterativeDeepeningDepth(Position& pos, int maxDepth, TranspositionTable& tt) {
    SearchResult best{};
    for (int d = 1; d <= maxDepth; ++d) {
        best = pvs(pos, d, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), pos.isWhiteToMove(), tt);
    }
    return best;
}

SearchResult iterativeDeepeningTime(Position& pos, int maxMillis, TranspositionTable& tt) {
    using namespace std::chrono;
    auto start = steady_clock::now();

    SearchResult best{};
    for (int d = 1; ; ++d) {
        auto now = steady_clock::now();
        auto elapsed = duration_cast<milliseconds>(now - start).count();
        if (elapsed >= maxMillis) break;

        best = pvs(pos, d, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), pos.isWhiteToMove(), tt);

        now = steady_clock::now();
        elapsed = duration_cast<milliseconds>(now - start).count();
        if (elapsed >= maxMillis) break;
    }
    return best;
}