#include "thread-pool.h"
#include <stdexcept>
#include <utility>

void ThreadPool::stop() {
    {
        std::lock_guard lock(m_mutex);
        m_stopping = true;
    }

    m_ready.notify_all();
    for(auto &worker : m_workers) {
        if(worker.joinable()) {
            worker.join();
        }
    }
}

ThreadPool::ThreadPool(unsigned count) {
    if(count == 0) {
        throw std::invalid_argument("Thread count must be positive");
    }

    try {
        for(unsigned i = 0; i < count; ++i) {
            m_workers.emplace_back([this] {
                while(true) {
                    std::packaged_task<void()> task;
                    {
                        std::unique_lock lock(m_mutex);
                        m_ready.wait(lock, [this] { return m_stopping || !m_tasks.empty(); });
                        if(m_tasks.empty()) {
                            return;
                        } 
                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }

                    task();
                }
            });
        } 
    } catch (...) {
        stop();
        throw;
    }
}

ThreadPool::~ThreadPool() {
    stop();
}
