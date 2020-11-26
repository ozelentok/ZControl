#include "FileCommandsHandler.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>

FileCommandsHandler::FileCommandsHandler() : _next_fd_id(0) {}

FileCommandsHandler::~FileCommandsHandler() {
	try {
		std::lock_guard<std::mutex> lock(_fds_mx);
		for (auto const &it : _fds) {
			::close(it.second);
		}
	} catch (...) {}
}

int32_t FileCommandsHandler::_get_fd(int32_t fd_id) {
	std::lock_guard<std::mutex> lock(_fds_mx);
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

Message FileCommandsHandler::access(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();
	const auto mode = deserializer.deserialize_int32();

	const int result = ::access(file_path.c_str(), mode);

	BinarySerializer serializer;
	serializer.serialize_uint8(result == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::rename(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto old_path = deserializer.deserialize_str();
	const auto new_path = deserializer.deserialize_str();

	const int value = ::rename(old_path.c_str(), new_path.c_str());

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::truncate(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();
	const auto size = deserializer.deserialize_int64();

	const int value = ::truncate(file_path.c_str(), size);

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::unlink(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();

	const int value = ::unlink(file_path.c_str());

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::chmod(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();
	const auto mode = deserializer.deserialize_int32();

	const int value = ::chmod(file_path.c_str(), mode);

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::chown(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();
	const auto owner = deserializer.deserialize_int32();
	const auto group = deserializer.deserialize_int32();

	const int value = ::chown(file_path.c_str(), owner, group);

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::utimens(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();
	struct timespec times[2];
	times[0].tv_sec = deserializer.deserialize_int64();
	times[0].tv_nsec = 0;
	times[1].tv_sec = deserializer.deserialize_int64();
	times[1].tv_nsec = 0;

	const int value = ::utimensat(AT_FDCWD, file_path.c_str(), times, 0);

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message FileCommandsHandler::open(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto file_path = deserializer.deserialize_str();
	const auto flags = deserializer.deserialize_int32();
	const auto mode = deserializer.deserialize_int32();

	const int32_t fd = ::open(file_path.c_str(), flags, mode);
	int32_t fd_id = -1;
	if (fd >= 0) {
		fd_id = _next_fd_id++;
		std::lock_guard<std::mutex> lock(_fds_mx);
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
	{
		std::lock_guard<std::mutex> lock(_fds_mx);
		_fds.erase(fd_id);
	}

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
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

Message FileCommandsHandler::pread(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();
	const auto size = deserializer.deserialize_uint32();
	const auto offset = deserializer.deserialize_int64();
	std::vector<uint8_t> buffer(size);

	const int32_t value = ::pread(_get_fd(fd_id), buffer.data(), size, offset);

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

Message FileCommandsHandler::pwrite(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();
	const auto offset = deserializer.deserialize_int64();
	const auto bytes = deserializer.deserialize_vector();

	const int32_t value = ::pwrite(_get_fd(fd_id), bytes.data(), bytes.size(), offset);

	BinarySerializer serializer;
	serializer.serialize_int32(value);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}
