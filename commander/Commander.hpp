#pragma once
#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include "DirEntry.hpp"
#include <atomic>
#include <map>
#include <future>
#include <thread>
#include <sys/stat.h>
#include <sys/statvfs.h>

class Commander {
	private:
		MessageTransport _transport;
		std::atomic_uint32_t _next_command_id;
		std::map<uint32_t, std::promise<Message>> _responses_promises;
		std::mutex _promises_mx;
		std::thread _responses_reader;
		bool _connected;

		Message _send_command(const Message &commander_msg);
		void _read_responses();

	public:
		Commander(TcpSocket &&connection);
		Commander(const Commander &other) = delete;
		Commander(Commander &&other) = delete;
		Commander& operator=(const Commander &other) = delete;
		Commander& operator=(Commander &&other) = delete;
		~Commander();
		bool is_connected() const;
		void disconnect();
		std::pair<bool, int32_t> getattr(const std::string &file_path, struct stat &file_info);
		std::pair<bool, int32_t> access(const std::string &file_path, int32_t mode);
		std::pair<bool, int32_t> rename(const std::string &old_path, const std::string &new_path, uint8_t flags);
		std::pair<bool, int32_t> truncate(const std::string &file_path, int64_t size);
		std::pair<bool, int32_t> unlink(const std::string &file_path);
		std::pair<bool, int32_t> chmod(const std::string &file_path, int32_t mode);
		std::pair<bool, int32_t> chown(const std::string &file_path, int32_t owner, int32_t group);
		std::pair<bool, int32_t> utimens(const std::string &file_path, const struct timespec times[2]);
		std::pair<bool, int32_t> statvfs(const std::string &file_path, struct statvfs *fs_info);
		std::pair<int32_t, int32_t> open(const std::string &file_path, int32_t flags, int32_t mode=DEFFILEMODE);
		std::pair<bool, int32_t> close(int32_t fd);
		std::pair<int32_t, int32_t> read(int32_t fd, uint8_t *bytes, uint32_t size);
		std::pair<int32_t, int32_t> pread(int32_t fd, uint8_t *bytes, uint32_t size, int64_t offset);
		std::pair<int32_t, int32_t> write(int32_t fd, const uint8_t *bytes, uint32_t size);
		std::pair<int32_t, int32_t> pwrite(int32_t fd, const uint8_t *bytes, uint32_t size, int64_t offset);
		std::pair<int32_t, int32_t> opendir(const std::string &dir_path);
		std::pair<bool, int32_t> closedir(int32_t fd);
		std::pair<std::vector<DirEntry>, int32_t> readdir(int32_t fd, uint32_t entries);
		std::pair<bool, int32_t> mkdir(const std::string &dir_path, int32_t mode);
		std::pair<bool, int32_t> rmdir(const std::string &dir_path);
};
