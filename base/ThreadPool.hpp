#pragma once
#include "ConcurrentQueue.hpp"
#include <thread>
#include <functional>

class ThreadPool {
	private:
		std::vector<std::thread> _threads;
		ConcurrentQueue<std::function<void()>> _queue;
		std::mutex _mx;
		void _poll_queue();

	public:
		ThreadPool(uint16_t pool_size);
		ThreadPool(const ThreadPool &other) = delete;
		ThreadPool(ThreadPool &&other) = delete;
		ThreadPool& operator=(const ThreadPool &other) = delete;
		ThreadPool& operator=(ThreadPool &&other) = delete;
		~ThreadPool();
		void submit(const std::function<void()> &func);
};
