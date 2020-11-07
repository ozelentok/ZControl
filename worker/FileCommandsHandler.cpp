#include "FileCommandsHandler.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

FileCommandsHandler::FileCommandsHandler() : _next_fd_id(0) {}

FileCommandsHandler::~FileCommandsHandler() {
	try {
		for (auto const &it : _fds) {
			::close(it.second);
		}
	} catch (...) {}
}

int32_t FileCommandsHandler::_get_fd(int32_t fd_id) const {
	auto entry = _fds.find(fd_id);
	if (entry == _fds.end()) {
		//TODO Log invalid fd_id in error log
		printf("Failed to find fd_id: %d\n", fd_id);
		return -1;
	}
	return entry->second;
}

Message FileCommandsHandler::getattr(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();

	struct stat file_info = { 0 };
	const int stat_result = ::stat(file_path.c_str(), &file_info);
	BinarySerializer serializer;
	serializer.serialize_uint8(stat_result == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	if (stat_result == 0) {
		serializer.serialize_uint32(file_info.st_mode);
		serializer.serialize_uint32(file_info.st_uid);
		serializer.serialize_uint32(file_info.st_gid);
		serializer.serialize_int64(file_info.st_size);
		serializer.serialize_int64(file_info.st_atim.tv_sec);
		serializer.serialize_int64(file_info.st_mtim.tv_sec);
		serializer.serialize_int64(file_info.st_ctim.tv_sec);
	}
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::open(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();
	const auto flags = deserializer.deserialize_uint32();

	const int32_t fd = ::open(file_path.c_str(), flags, DEFFILEMODE);
	int32_t fd_id = -1;
	if (fd >= 0) {
		//TODO - use atomic counter
		fd_id = _next_fd_id++;
		//TODO - use lock
		_fds.emplace(fd_id, fd);
	}

	BinarySerializer serializer;
	serializer.serialize_int32(fd_id);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::close(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();

	const int32_t value = ::close(_get_fd(fd_id));
	_fds.erase(fd_id);

	BinarySerializer serializer;
	serializer.serialize_int32(value);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::read(const Message& message) {
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

Message FileCommandsHandler::write(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();
	const auto bytes = deserializer.deserialize_vector();

	const int32_t value = ::write(_get_fd(fd_id), bytes.data(), bytes.size());

	BinarySerializer serializer;
	serializer.serialize_int32(value);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}