#include "Commander.hpp"

Commander::Commander(const std::string &host, uint16_t port) 
	: _connection(), _transport(_connection), _command_next_id(0) {
		_connection.connect(host, port);
}

int Commander::open(const std::string &file_path, int32_t flags) {
	Message m;
	m.id = _command_next_id++;
	m.type = CommanderMessageType::Open;
	m.
	m.length = 0;
}

int Commander::close(uint32_t fd) {
	
}

size_t Commander::read(uint8_t *bytes, size_t size) {
	
}

void Commander::write(uint32_t fd, const uint8_t *bytes, size_t size) {
	
}
