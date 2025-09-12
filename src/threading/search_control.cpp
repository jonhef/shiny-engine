#include "search_control.h"

namespace search_control {
std::atomic<bool> stopSearch{false};
clock_t::time_point deadline = clock_t::now();
}
