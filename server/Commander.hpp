#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include <map>

class Commander {
	private:
		TcpSocket _connection;
		MessageTransport _transport;
		uint32_t _command_next_id;

	public:
		Commander(const std::string &host, uint16_t port);
		//TODO - use DEFFILEMODE  as 3rd parameter in worker
		int open(const std::string &file_path, int32_t flags);
		int close(uint32_t fd);
		size_t read(uint8_t *bytes, size_t size);
		void write(uint32_t fd, const uint8_t *bytes, size_t size);
};
