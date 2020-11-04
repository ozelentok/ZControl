#pragma once
#include "MessageTransport.hpp"
#include <map>
#include <sys/types.h>
#include <dirent.h>

class DirCommandsHandler {
	private:
		std::map<int32_t, DIR*> _fds;
		int32_t  _next_fd_id;
		DIR* _get_fd(int32_t fd_id) const;

	public:
		DirCommandsHandler();
		DirCommandsHandler(const DirCommandsHandler&) = delete;
		DirCommandsHandler(DirCommandsHandler&&) = delete;
		~DirCommandsHandler();
		Message opendir(const Message& message);
		Message readdir(const Message& message);
		Message closedir(const Message& message);
};
