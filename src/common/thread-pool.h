#pragma once
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <stdexcept>
#include <utility>

class ThreadPool {
public:
    explicit ThreadPool(unsigned count);

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    ~ThreadPool();

    template<typename T>
    std::future<T> post(std::function<T()> function) {
        std::packaged_task<T()> task(std::move(function));
        auto result = task.get_future();
        std::packaged_task<void()> work([task = std::move(task)]() mutable {
            task();
        });

        {
            std::lock_guard lock(m_mutex);
            if(m_stopping) {
                throw std::runtime_error("Thread pool is stopping");
            }
            m_tasks.push(std::move(work));
        }

        m_ready.notify_one();
        return result;
    }

private:
    void stop();

    std::mutex m_mutex;
    std::condition_variable m_ready;
    std::queue<std::packaged_task<void()>> m_tasks;
    bool m_stopping = false;
    std::vector<std::thread> m_workers;
};
