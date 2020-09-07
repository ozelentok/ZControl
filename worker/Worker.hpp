#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include <map>

class Worker {
	private:
		TcpSocket _connection;
		MessageTransport _transport;
		std::map<int32_t, int32_t> _fds;
		int32_t  _next_fd_id;

		int32_t _get_fd(int32_t fd_id) const;

	public:
		Worker(const std::string &host, uint16_t port);
		void work();

		//TODO - use DEFFILEMODE  as 3rd parameter in worker
		Message open(const std::string &file_path, int32_t flags);
		Message close(uint32_t fd);
		Message read(uint32_t fd, uint8_t *bytes, uint32_t size);
		Message write(uint32_t fd, const uint8_t *bytes, uint32_t size);
};
