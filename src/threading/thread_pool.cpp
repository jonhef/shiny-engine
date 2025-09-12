#include "thread_pool.h"

ThreadPool::ThreadPool(size_t nthreads) {
    if (nthreads == 0) nthreads = 1;
    for (size_t i = 0; i < nthreads; ++i) {
        workers.emplace_back([this]() { this->workerLoop(); });
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

void ThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [this]{ return stop.load() || !tasks.empty(); });
            if (stop.load() && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        try { task(); } catch (...) { /* optional logging */ }
    }
}

void ThreadPool::enqueue(std::function<void()> job) {
    {
        std::lock_guard<std::mutex> lk(mtx);
        tasks.emplace(std::move(job));
    }
    cv.notify_one();
}

void ThreadPool::shutdown() {
    bool expected = false;
    if (!stop.compare_exchange_strong(expected, true)) return;
    cv.notify_all();
    for (auto &t : workers) if (t.joinable()) t.join();
}
