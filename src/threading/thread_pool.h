#pragma once
#include <functional>
#include <future>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <utility>
#include <type_traits>

class ThreadPool {
public:
    explicit ThreadPool(size_t nthreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    // submit task, returns future<R>
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
        using Ret = typename std::invoke_result_t<F, Args...>;

        auto taskPtr = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<Ret> res = taskPtr->get_future();
        {
            std::lock_guard<std::mutex> lk(mtx);
            tasks.emplace([taskPtr]() {
                try { (*taskPtr)(); }
                catch(...) { /* swallow or log */ }
            });
        }
        cv.notify_one();
        return res;
    }

    // simple enqueue: submit a void() task; no futures involved
    void enqueue(std::function<void()> job);

    void shutdown(); // graceful

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> stop{false};

    void workerLoop();
};
