#pragma once
#include "MessageTransport.hpp"
#include <map>
#include <mutex>
#include <atomic>
#include <sys/types.h>
#include <dirent.h>

class DirCommandsHandler {
	private:
		std::map<int32_t, DIR*> _fds;
		std::mutex _fds_mx;
		std::atomic_uint32_t _next_fd_id;
		DIR* _get_fd(int32_t fd_id);

	public:
		DirCommandsHandler();
		DirCommandsHandler(const DirCommandsHandler &other) = delete;
		DirCommandsHandler(DirCommandsHandler &&other) = delete;
		DirCommandsHandler& operator=(const DirCommandsHandler &other) = delete;
		DirCommandsHandler& operator=(DirCommandsHandler &&other) = delete;
		~DirCommandsHandler();
		Message opendir(const Message& message);
		Message readdir(const Message& message);
		Message closedir(const Message& message);
		Message mkdir(const Message& message);
		Message rmdir(const Message& message);
};
