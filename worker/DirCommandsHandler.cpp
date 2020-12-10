#include "DirCommandsHandler.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>

DirCommandsHandler::DirCommandsHandler() : _next_fd_id(0) {}

DirCommandsHandler::~DirCommandsHandler() {
	try {
		std::lock_guard<std::mutex> lock(_fds_mx);
		for (auto const &it : _fds) {
			::closedir(it.second);
		}
	} catch (...) {}
}

DIR* DirCommandsHandler::_get_fd(int32_t fd_id) {
	std::lock_guard<std::mutex> lock(_fds_mx);
	auto entry = _fds.find(fd_id);
	if (entry == _fds.end()) {
		syslog(LOG_WARNING, "Server used an invalid directory descriptor id: %d", fd_id);
		return nullptr;
	}
	return entry->second;
}

Message DirCommandsHandler::opendir(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto dir_path = deserializer.deserialize_str();

	DIR *fd = ::opendir(dir_path.c_str());
	int32_t fd_id = -1;
	if (fd != nullptr) {
		fd_id = _next_fd_id++;
		std::lock_guard<std::mutex> lock(_fds_mx);
		_fds.emplace(fd_id, fd);
	}

	BinarySerializer serializer;
	serializer.serialize_int32(fd_id);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message DirCommandsHandler::closedir(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();

	const int32_t value = ::closedir(_get_fd(fd_id));
	{
		std::lock_guard<std::mutex> lock(_fds_mx);
		_fds.erase(fd_id);
	}

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message DirCommandsHandler::readdir(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto fd_id = deserializer.deserialize_int32();
	const auto entries = deserializer.deserialize_uint32();

	BinarySerializer serializer;
	const auto fd = _get_fd(fd_id);
	if (fd != nullptr) {
		for (uint32_t i = 0; i < entries; i++) {
			errno = 0;
			const dirent *dir = ::readdir(fd);
			if (dir == nullptr) {
				break;
			}
			serializer.serialize_uint32(dir->d_ino);
			serializer.serialize_uint8(dir->d_type);
			serializer.serialize_str(dir->d_name);
		}
		serializer.serialize_int32(errno);
	} else {
		serializer.serialize_int32(EBADF);
	}
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message DirCommandsHandler::mkdir(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto dir_path = deserializer.deserialize_str();
	const auto mode = deserializer.deserialize_int32();

	const int32_t value = ::mkdir(dir_path.c_str(), mode);

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message DirCommandsHandler::rmdir(const Message& message) {
	BinaryDeserializer deserializer(message.data);
	const auto dir_path = deserializer.deserialize_str();

	const int32_t value = ::rmdir(dir_path.c_str());

	BinarySerializer serializer;
	serializer.serialize_uint8(value == 0 ? 1 : 0);
	serializer.serialize_int32(errno);
	return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}
