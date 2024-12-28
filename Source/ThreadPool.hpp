#pragma once

#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
 public:
  ThreadPool(std::size_t numThreads = std::thread::hardware_concurrency()) {
    m_threads.reserve(numThreads);

    for (std::size_t i = 0; i < numThreads; ++i) {
      m_threads.emplace_back([this] {
        while (true) {
          std::function<void()> task;

          {
            std::unique_lock lock(m_mutex);
            m_condition.wait(lock, [this] { return !m_tasks.empty() || m_stop; });

            if (m_stop && m_tasks.empty()) {
              return;
            }

            task = std::move(m_tasks.front());
            m_tasks.pop();
          }

          task();
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_stop = true;
    }

    m_condition.notify_all();

    for (auto& thread : m_threads) {
      thread.join();
    }
  }

  void push(std::function<void()> task) {
    if (m_stop) {
      throw std::runtime_error("Cannot add task to stopped thread pool");
    }

    {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_tasks.emplace(std::move(task));
    }

    m_condition.notify_one();
  }

 private:
  bool m_stop{false};
  std::vector<std::thread> m_threads;
  std::mutex m_mutex;
  std::condition_variable m_condition;
  std::queue<std::function<void()>> m_tasks;
};
