#include "MessageTransport.hpp"
#include <cstring>
#include <stdexcept>

MessageTransport::MessageTransport(TcpSocket &socket) :_socket(socket) {}

void MessageTransport::_read_exactly(uint8_t* buffer, uint32_t count) {
	uint32_t bytes_read = 0;
	uint32_t total_bytes_read = 0;
	while (total_bytes_read < count) {
		bytes_read = _socket.recv(buffer + total_bytes_read, count - total_bytes_read);
		if (bytes_read == 0) {
			//TODO: Handle connection close
			throw std::runtime_error("Connection closed");
		}
		total_bytes_read += bytes_read;
	}
}

void MessageTransport::read(Message &message) {
	//TODO: Handle little/big endian
	uint8_t message_header[sizeof(message.id)+sizeof(message.type)+sizeof(message.length)] = { 0 };
	_read_exactly(message_header, sizeof(message_header));
	::memcpy(&message, message_header, sizeof(message_header));
	message.data.reserve(message.length);
	_read_exactly(message.data.data(), message.length);
}

void MessageTransport::write(const Message& message) {
	//TODO: Handle little/big endian
	_socket.send(reinterpret_cast<const uint8_t*>(&message),
							 sizeof(message.id)+sizeof(message.type)+sizeof(message.length));
	_socket.send(message.data.data(), message.length);
}
