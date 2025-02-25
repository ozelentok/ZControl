#pragma once
#include "Logger.hpp"

#include <queue>
#include <mutex>
#include <condition_variable>
#include <stdexcept>

class QueueShutdown : public std::runtime_error {
public:
  QueueShutdown() : std::runtime_error("Queue Shutdown") {}
};

template <typename T> class ConcurrentQueue {
private:
  std::queue<T> _queue;
  std::mutex _mx;
  std::condition_variable _cv;
  bool _shutdown;

public:
  ConcurrentQueue() : _shutdown(false) {};
  ConcurrentQueue(const ConcurrentQueue &other) = delete;
  ConcurrentQueue(ConcurrentQueue &&other) = delete;
  ConcurrentQueue &operator=(const ConcurrentQueue &other) = delete;
  ConcurrentQueue &operator=(ConcurrentQueue &&other) = delete;
  ~ConcurrentQueue() {
    DTOR_TRY
    shutdown();
    DTOR_CATCH
  }

  bool empty() {
    std::lock_guard<std::mutex> lock(_mx);
    return _queue.empty();
  }

  void push(const T &value) {
    std::lock_guard<std::mutex> lock(_mx);
    _queue.push(value);
    _cv.notify_one();
  }

  void push(T &&value) {
    std::lock_guard<std::mutex> lock(_mx);
    _queue.push(std::forward<T>(value));
    _cv.notify_one();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(_mx);
    _cv.wait(lock, [this] { return !_queue.empty() || _shutdown; });
    if (_shutdown) {
      throw QueueShutdown();
    }
    T value = std::move(_queue.front());
    _queue.pop();
    return value;
  }

  void shutdown() {
    if (_shutdown) {
      return;
    }

    _shutdown = true;
    std::lock_guard<std::mutex> lock(_mx);
    _cv.notify_all();
  }
};
