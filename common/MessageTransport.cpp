#include "MessageTransport.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"

Message::Message(uint32_t id, uint8_t type, const std::vector<uint8_t>&& data) :
	id(id), type(type), data(data) {}

MessageTransport::MessageTransport(const std::string &host, uint16_t port) {
	_socket.connect(host, port);
}

MessageTransport::MessageTransport(TcpSocket &&socket) :_socket(std::move(socket)) {}

void MessageTransport::_read_exactly(uint8_t* buffer, uint32_t count) {
	uint32_t bytes_read = 0;
	uint32_t total_bytes_read = 0;
	while (total_bytes_read < count) {
		bytes_read = _socket.recv(buffer + total_bytes_read, count - total_bytes_read);
		if (bytes_read == 0) {
			throw TransportClosed();
		}
		total_bytes_read += bytes_read;
	}
}

Message MessageTransport::read() {
	std::unique_lock<std::mutex> lock(_read_mx);
	std::vector<uint8_t> header_buffer(sizeof(uint32_t) + sizeof(uint8_t) + sizeof(uint32_t));
	_read_exactly(header_buffer.data(), header_buffer.size());

	BinaryDeserializer header_deserializer(header_buffer);
	auto id = header_deserializer.deserialize_uint32();
	auto type = header_deserializer.deserialize_uint8();
	auto data_size = header_deserializer.deserialize_uint32();

	std::vector<uint8_t> msg_buffer(data_size);
	_read_exactly(msg_buffer.data(), msg_buffer.size());
	lock.unlock();
	BinaryDeserializer msg_deserializer(msg_buffer);
	auto data = msg_deserializer.deserialize_vector(data_size);
	return Message(id, type, std::move(data));
}

void MessageTransport::write(const Message& message) {
	BinarySerializer serializer;
	serializer.serialize_uint32(message.id);
	serializer.serialize_uint8(message.type);
	serializer.serialize_vector(message.data);
	std::lock_guard<std::mutex> lock(_write_mx);
	_socket.send(serializer.data());
}

void MessageTransport::close() {
	_socket.close();
}
