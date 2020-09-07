#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include <map>

class Commander {
	private:
		MessageTransport _transport;
		uint32_t _command_next_id;
		uint32_t _last_errno;

	public:
		Commander(TcpSocket &connection);
		uint32_t last_errno() const;
		//TODO - use DEFFILEMODE  as 3rd parameter in worker
		int32_t open(const std::string &file_path, int32_t flags);
		int32_t close(int32_t fd);
		int32_t read(int32_t fd, uint8_t *bytes, uint32_t size);
		int32_t write(int32_t fd, const uint8_t *bytes, uint32_t size);
};
