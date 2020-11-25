#pragma once
#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include "DirEntry.hpp"
#include <atomic>
#include <map>
#include <future>
#include <thread>
#include <sys/stat.h>

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
		std::pair<int32_t, int32_t> open(const std::string &file_path, int32_t flags, int32_t mode=DEFFILEMODE);
		std::pair<int32_t, int32_t> close(int32_t fd);
		std::pair<int32_t, int32_t> read(int32_t fd, uint8_t *bytes, uint32_t size);
		std::pair<int32_t, int32_t> pread(int32_t fd, uint8_t *bytes, uint32_t size, uint64_t offset);
		std::pair<int32_t, int32_t> write(int32_t fd, const uint8_t *bytes, uint32_t size);
		std::pair<int32_t, int32_t> pwrite(int32_t fd, const uint8_t *bytes, uint32_t size, uint64_t offset);
		std::pair<int32_t, int32_t> opendir(const std::string &dir_path);
		std::pair<int32_t, int32_t> closedir(int32_t fd);
		std::pair<std::vector<DirEntry>, int32_t> readdir(int32_t fd, uint32_t entries);
};
