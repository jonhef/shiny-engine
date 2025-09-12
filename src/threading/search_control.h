#pragma once
#include <atomic>
#include <chrono>

namespace search_control {

using clock_t = std::chrono::steady_clock;
extern std::atomic<bool> stopSearch;
extern clock_t::time_point deadline;

// Установить дедлайн (текущее время + millis)
inline void setDeadlineMillis(int64_t millis) {
    deadline = clock_t::now() + std::chrono::milliseconds(millis);
    stopSearch.store(false);
}

// Проверить пора ли остановиться
inline bool shouldStop() {
    if (stopSearch.load(std::memory_order_relaxed)) return true;
    return clock_t::now() >= deadline;
}

// Принудительно остановить
inline void requestStop() {
    stopSearch.store(true);
}
} // namespace search_control
