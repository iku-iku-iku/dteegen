#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
class ThreadPool {
public:
  explicit ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
      workers_.emplace_back([this] {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex_);
            this->condition_.wait(
                lock, [this] { return this->stop_ || !this->tasks_.empty(); });
            if (this->stop_ && this->tasks_.empty()) {
              return;
            }
            task = std::move(this->tasks_.front());
            this->tasks_.pop();
          }
          task();
        }
      });
    }
  }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      stop_ = true;
    }
    condition_.notify_all();
    for (std::thread &worker : workers_) {
      worker.join();
    }
  }

  void enqueue(std::function<void()> task) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      tasks_.emplace(task);
    }
    condition_.notify_one();
  }

private:
  std::vector<std::thread> workers_;
  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::queue<std::function<void()>> tasks_;
  bool stop_ = false;
};
