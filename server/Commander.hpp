#pragma once
#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include "DirEntry.hpp"
#include <map>
#include <future>
#include <thread>

class Commander {
	private:
		MessageTransport _transport;
		uint32_t _command_next_id;
		uint32_t _last_errno;
		std::map<uint32_t, std::promise<Message>> _responses_promises;
		std::thread _responses_reader;
		bool _connected;

		Message _send_command(const Message &);
		void _read_responses();

	public:
		Commander(TcpSocket &connection);
		Commander(const Commander&) = delete;
		~Commander();
		uint32_t last_errno() const;
		void disconnect();
		int32_t open(const std::string &file_path, int32_t flags);
		int32_t close(int32_t fd);
		int32_t read(int32_t fd, uint8_t *bytes, uint32_t size);
		int32_t write(int32_t fd, const uint8_t *bytes, uint32_t size);
		int32_t opendir(const std::string &dir_path);
		int32_t closedir(int32_t fd);
		std::vector<DirEntry> readdir(int32_t fd, uint32_t entries);
};
