#include "Commander.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <cstring>

Commander::Commander(TcpSocket &connection) 
	: _transport(connection), _command_next_id(0), _last_errno(0) {
}

int32_t Commander::open(const std::string &file_path, int32_t flags) {
	Message m;
	m.id = _command_next_id++;
	m.type.commander = CommanderMessageType::Open;

	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_uint32(flags);

	m.data = serializer.data();
	m.length = m.data.size();

	_transport.write(m);
	_transport.read(m);

	BinaryDeserializer deserializer(m.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

int32_t Commander::close(int32_t fd) {
	Message m;
	m.id = _command_next_id++;
	m.type.commander = CommanderMessageType::Close;

	BinarySerializer serializer;
	serializer.serialize_int32(fd);

	m.data = serializer.data();
	m.length = m.data.size();

	_transport.write(m);
	_transport.read(m);

	BinaryDeserializer deserializer(m.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

int32_t Commander::read(int32_t fd, uint8_t *bytes, uint32_t size) {
	Message m;
	m.id = _command_next_id++;
	m.type.commander = CommanderMessageType::Read;

	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(size);

	m.data = serializer.data();
	m.length = m.data.size();

	_transport.write(m);
	_transport.read(m);

	BinaryDeserializer deserializer(m.data);

	int32_t value = deserializer.deserialize_int32();
	uint32_t last_errno = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = last_errno;
	} else {
		auto bytes_vector = deserializer.deserialize_vector();
		//TODO: Possible overflow
		::memcpy(bytes, bytes_vector.data(), value);
	}
	return value;
}

int32_t Commander::write(int32_t fd, const uint8_t *bytes, uint32_t size) {
	Message m;
	m.id = _command_next_id++;
	m.type.commander = CommanderMessageType::Write;

	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(size);
	for (uint32_t i = 0; i < size; i++) {
		serializer.serialize_uint8(bytes[i]);
	}

	m.data = serializer.data();
	m.length = m.data.size();

	_transport.write(m);
	_transport.read(m);

	BinaryDeserializer deserializer(m.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}
