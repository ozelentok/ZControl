#pragma once
#include "MessageTransport.hpp"
#include <map>
#include <mutex>
#include <atomic>

class FileCommandsHandler {
	private:
		std::map<int32_t, int32_t> _fds;
		std::mutex _fds_mx;
		std::atomic_uint32_t _next_fd_id;
		int32_t _get_fd(int32_t fd_id);

	public:
		FileCommandsHandler();
		FileCommandsHandler(const FileCommandsHandler &other) = delete;
		FileCommandsHandler(FileCommandsHandler &&other) = delete;
		FileCommandsHandler& operator=(const FileCommandsHandler &other) = delete;
		FileCommandsHandler& operator=(FileCommandsHandler &&other) = delete;
		~FileCommandsHandler();
		Message getattr(const Message& message);
		Message access(const Message& message);
		Message open(const Message& message);
		Message close(const Message& message);
		Message read(const Message& message);
		Message pread(const Message& message);
		Message write(const Message& message);
		Message pwrite(const Message& message);
};
