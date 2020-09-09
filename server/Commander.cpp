#include "Commander.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <stdexcept>
#include <cstring>

Commander::Commander(TcpSocket &connection) 
	: _transport(connection), _command_next_id(0), _last_errno(0) {
}

int32_t Commander::open(const std::string &file_path, int32_t flags) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_uint32(flags);

	auto commander_msg = Message(_command_next_id++, CommanderMessageType::Open, serializer.data());
	_transport.write(commander_msg);
	auto worker_msg = _transport.read();
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

int32_t Commander::close(int32_t fd) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);

	auto commander_msg = Message(_command_next_id++, CommanderMessageType::Close, serializer.data());
	_transport.write(commander_msg);
	auto worker_msg = _transport.read();
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

int32_t Commander::read(int32_t fd, uint8_t *bytes, uint32_t size) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(size);

	auto commander_msg = Message(_command_next_id++, CommanderMessageType::Read, serializer.data());
	_transport.write(commander_msg);
	auto worker_msg = _transport.read();
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	uint32_t last_errno = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = last_errno;
		return value;
	} 

	auto bytes_vector = deserializer.deserialize_vector();
	if (bytes_vector.size() > size) {
		throw std::runtime_error("read returned more bytes than requested");
	}

	::memcpy(bytes, bytes_vector.data(), value);
	return value;
}

int32_t Commander::write(int32_t fd, const uint8_t *bytes, uint32_t size) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(size);
	for (uint32_t i = 0; i < size; i++) {
		serializer.serialize_uint8(bytes[i]);
	}

	auto commander_msg = Message(_command_next_id++, CommanderMessageType::Write, serializer.data());
	_transport.write(commander_msg);
	auto worker_msg = _transport.read();
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}
