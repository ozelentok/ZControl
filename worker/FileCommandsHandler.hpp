#pragma once
#include "MessageTransport.hpp"
#include <map>

class FileCommandsHandler {
	private:
		std::map<int32_t, int32_t> _fds;
		int32_t  _next_fd_id;
		int32_t _get_fd(int32_t fd_id) const;

	public:
		FileCommandsHandler();
		FileCommandsHandler(const FileCommandsHandler&) = delete;
		FileCommandsHandler(FileCommandsHandler&&) = delete;
		~FileCommandsHandler();
		Message open(const Message& message);
		Message close(const Message& message);
		Message read(const Message& message);
		Message write(const Message& message);
};
