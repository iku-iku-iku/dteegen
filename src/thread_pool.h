#pragma once

#include "parser.h"
#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
class ThreadPool {
public:
  std::mutex tls_mtx;
  std::unordered_map<std::thread::id, std::vector<FunctionInfo> *>
      g_secure_entry_func_list_map;
  std::unordered_map<std::thread::id, std::vector<FunctionInfo> *>
      g_insecure_entry_func_list_map;
  std::unordered_map<std::thread::id, std::unordered_set<FuncName> *>
      g_func_calls_in_insecure_world_map;
  std::unordered_map<std::thread::id, std::unordered_set<FuncName> *>
      g_func_calls_in_secure_world_map;
  explicit ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; ++i) {
      workers_.emplace_back([this] {
        {
          std::scoped_lock<std::mutex> lock(tls_mtx);
          g_secure_entry_func_list_map[std::this_thread::get_id()] =
              &tls_secure_entry_func_list;
          g_insecure_entry_func_list_map[std::this_thread::get_id()] =
              &tls_insecure_entry_func_list;
          g_func_calls_in_insecure_world_map[std::this_thread::get_id()] =
              &tls_func_calls_in_insecure_world;
          g_func_calls_in_secure_world_map[std::this_thread::get_id()] =
              &tls_func_calls_in_secure_world;
        }
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
          --task_unfinished_count_;
          if (tasks_.empty()) {
            empty_.notify_all();
          }
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
      ++task_unfinished_count_;
    }
    condition_.notify_one();
  }

  void wait_queue_empty() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    empty_.wait(
        lock, [this] { return task_unfinished_count_ == 0 && tasks_.empty(); });
    // collect tls
    for (auto &[_, tls] : g_secure_entry_func_list_map) {
      g_secure_entry_func_list.insert(g_secure_entry_func_list.end(),
                                      tls->begin(), tls->end());
      tls->clear();
    }
    for (auto &[_, tls] : g_insecure_entry_func_list_map) {
      g_insecure_entry_func_list.insert(g_insecure_entry_func_list.end(),
                                        tls->begin(), tls->end());
      tls->clear();
    }
    for (auto &[_, tls] : g_func_calls_in_insecure_world_map) {
      g_func_calls_in_insecure_world.insert(tls->begin(), tls->end());
      tls->clear();
    }
    for (auto &[_, tls] : g_func_calls_in_secure_world_map) {
      g_func_calls_in_secure_world.insert(tls->begin(), tls->end());
      tls->clear();
    }
  }

private:
  std::vector<std::thread> workers_;
  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::condition_variable empty_;
  std::queue<std::function<void()>> tasks_;
  std::atomic<int> task_unfinished_count_ = 0;
  bool stop_ = false;
};
