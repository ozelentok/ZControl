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
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		~ThreadPool();
		void submit(const std::function<void()> &func);
};
