#pragma once
#include "ConcurrentQueue.hpp"
#include <thread>
#include <functional>

class ThreadPool {
private:
  ConcurrentQueue<std::function<void()>> _queue;
  std::vector<std::jthread> _threads;
  void _poll_queue();

public:
  ThreadPool(uint16_t pool_size);
  ThreadPool(const ThreadPool &other) = delete;
  ThreadPool(ThreadPool &&other) = delete;
  ThreadPool &operator=(const ThreadPool &other) = delete;
  ThreadPool &operator=(ThreadPool &&other) = delete;
  ~ThreadPool();
  void submit(const std::function<void()> &func);
  uint16_t pool_size() const;
};
