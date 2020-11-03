#pragma once
#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include "ThreadPool.hpp"
#include <map>

class Worker {
	private:
		TcpSocket _connection;
		MessageTransport _transport;
		ThreadPool _thread_pool;
		std::map<int32_t, int32_t> _fds;
		int32_t  _next_fd_id;
		bool _should_disconnect;

		int32_t _get_fd(int32_t fd_id) const;
		Message _disconnect(const Message& message);
		Message _open(const Message& message);
		Message _close(const Message& message);
		Message _read(const Message& message);
		Message _write(const Message& message);
		void _handle_commander_message(const Message &commander_msg);

	public:
		Worker(const std::string &host, uint16_t port);
		Worker(Worker&) = delete;
		void work();
};
