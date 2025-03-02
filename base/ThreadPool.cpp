#include "ThreadPool.hpp"

ThreadPool::ThreadPool(uint16_t pool_size) {
  for (uint16_t i = 0; i < pool_size; i++) {
    _threads.push_back(std::thread(&ThreadPool::_poll_queue, this));
  }
}

ThreadPool::~ThreadPool() {
  DTOR_TRY
  _queue.shutdown();
  for (auto &t : _threads) {
    t.join();
  }
  DTOR_CATCH
}

void ThreadPool::submit(const std::function<void()> &func) {
  _queue.push(func);
}

void ThreadPool::_poll_queue() {
  try {
    while (true) {
      _queue.pop()();
    }
  } catch (const QueueShutdown &) {
  }
}

uint16_t ThreadPool::pool_size() const {
  return _threads.size();
}
