#include "Worker.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

Worker::Worker(const std::string &host, uint16_t port) :
	_connection(), _transport(_connection), _next_fd_id(0), _should_disconnect(false) {
	_connection.connect(host, port);
}

int32_t Worker::_get_fd(int32_t fd_id) const {
	auto entry = _fds.find(fd_id);
	if (entry == _fds.end()) {
		//TODO Log invalid fd_id in error log
		printf("Failed to find fd_id: %d\n", fd_id);
		return -1;
	}
	return entry->second;
}

void Worker::work() {
	while (!_should_disconnect) {
		Message commander_msg = _transport.read();
		printf("Got message: id: %d, type: %d, data_length: %ld\n",
					 commander_msg.id, commander_msg.type, commander_msg.data.size());

		switch (commander_msg.type) {
			case CommanderMessageType::Disconnect:
				_transport.write(_disconnect(commander_msg));
				break;
			case CommanderMessageType::Open:
				_transport.write(_open(commander_msg));
				break;
			case CommanderMessageType::Close:
				_transport.write(_close(commander_msg));
				break;
			case CommanderMessageType::Read:
				_transport.write(_read(commander_msg));
				break;
			case CommanderMessageType::Write:
				_transport.write(_write(commander_msg));
				break;
		}
	}
}

Message Worker::_disconnect(const Message& message) {
	_should_disconnect = true;
	printf("got disconnect message\n");
	return Message(message.id, WorkerMessageType::CommandResult, std::vector<uint8_t>(0));
}

Message Worker::_open(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();
	const auto flags = deserializer.deserialize_uint32();

	const int32_t fd = ::open(file_path.c_str(), flags, DEFFILEMODE);
	int32_t fd_id = -1;
	if (fd >= 0) {
		fd_id = _next_fd_id++;
		_fds.emplace(fd_id, fd);
	}

	BinarySerializer serializer;
	serializer.serialize_int32(fd_id);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message Worker::_close(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();

	const int32_t value = ::close(_get_fd(fd_id));
	_fds.erase(fd_id);

	BinarySerializer serializer;
	serializer.serialize_int32(value);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message Worker::_read(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();
	const auto size = deserializer.deserialize_uint32();
	std::vector<uint8_t> buffer(size);

	const int32_t value = ::read(_get_fd(fd_id), buffer.data(), size);

	BinarySerializer serializer;
	serializer.serialize_int32(value);
	serializer.serialize_int32(errno);
	if (value >= 0) {
		buffer.resize(value);
	}
	serializer.serialize_vector(buffer);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message Worker::_write(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();
	const auto bytes = deserializer.deserialize_vector();

	const int32_t value = ::write(_get_fd(fd_id), bytes.data(), bytes.size());

	BinarySerializer serializer;
	serializer.serialize_int32(value);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}
