#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include <map>

class Worker {
	private:
		TcpSocket _connection;
		MessageTransport _transport;
		std::map<int32_t, int32_t> _fds;
		int32_t  _next_fd_id;
		bool _should_disconnect;

		int32_t _get_fd(int32_t fd_id) const;

	public:
		Worker(const std::string &host, uint16_t port);
		void work();

		Message _disconnect(const Message& message);
		Message _open(const Message& message);
		Message _close(const Message& message);
		Message _read(const Message& message);
		Message _write(const Message& message);
};
