#include "Commander.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <stdexcept>
#include <cstring>
#include <sys/stat.h>

Commander::Commander(TcpSocket &&connection)
	: _transport(std::move(connection)), _next_command_id(0), _last_errno(0),
	_responses_reader(&Commander::_read_responses, this),
	_connected(true) {
}

Commander::~Commander() {
	try {
		disconnect();
		_transport.close();
		_responses_reader.join();
	} catch (...) {
	}
}

int32_t Commander::last_errno() const {
	return _last_errno;
}

void Commander::_read_responses() {
	while (_connected) {
		try {
			auto worker_msg = _transport.read();
			auto promise = std::move(_responses_promises.at(worker_msg.id));
			promise.set_value(worker_msg);
			_responses_promises.erase(worker_msg.id);
		} catch (...) {
			_connected = false;
			for (auto it = _responses_promises.begin(); it != _responses_promises.end(); ++it) {
				it->second.set_exception(std::current_exception());
			}
		}
	}
}

Message Commander::_send_command(const Message &commander_msg) {
	_transport.write(commander_msg);
	std::promise<Message> promise;
	auto future = promise.get_future();

	_responses_promises[commander_msg.id] = std::move(promise);
	Message worker_msg = future.get();

	return worker_msg;
}

void Commander::disconnect() {
	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Disconnect, std::vector<uint8_t>(0));
	_send_command(commander_msg);
	_connected = false;
}

bool Commander::getattr(const std::string &file_path, struct stat &file_info) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::GetAtr, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	uint32_t last_errno = deserializer.deserialize_int32();
	if (!value) {
		_last_errno = last_errno;
	}	else {
		file_info.st_mode = deserializer.deserialize_uint32();
		file_info.st_uid = deserializer.deserialize_uint32();
		file_info.st_gid = deserializer.deserialize_uint32();
		file_info.st_size = deserializer.deserialize_int64();
		file_info.st_atim.tv_sec = deserializer.deserialize_int64();
		file_info.st_mtim.tv_sec = deserializer.deserialize_int64();
		file_info.st_ctim.tv_sec = deserializer.deserialize_int64();
	}
	return value;
}

bool Commander::access(const std::string &file_path, int32_t mode) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_int32(mode);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Access, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	if (!value) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

int32_t Commander::open(const std::string &file_path, int32_t flags, int32_t mode) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_int32(flags);
	serializer.serialize_int32(mode);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Open, serializer.data());
	Message worker_msg = _send_command(commander_msg);
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

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Close, serializer.data());
	Message worker_msg = _send_command(commander_msg);
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

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Read, serializer.data());
	Message worker_msg = _send_command(commander_msg);
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

int32_t Commander::pread(int32_t fd, uint8_t *bytes, uint32_t size, uint64_t offset) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(size);
	serializer.serialize_int64(offset);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::PRead, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	uint32_t last_errno = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = last_errno;
		return value;
	}

	auto bytes_vector = deserializer.deserialize_vector();
	if (bytes_vector.size() > size) {
		throw std::runtime_error("pread returned more bytes than requested");
	}

	::memcpy(bytes, bytes_vector.data(), value);
	return value;
}

int32_t Commander::write(int32_t fd, const uint8_t *bytes, uint32_t size) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(size);
	//TODO: Serealize bytes array
	for (uint32_t i = 0; i < size; i++) {
		serializer.serialize_uint8(bytes[i]);
	}

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Write, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

int32_t Commander::pwrite(int32_t fd, const uint8_t *bytes, uint32_t size, uint64_t offset) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_int64(offset);
	serializer.serialize_uint32(size);
	//TODO: Serealize bytes array
	for (uint32_t i = 0; i < size; i++) {
		serializer.serialize_uint8(bytes[i]);
	}

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::PWrite, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

int32_t Commander::opendir(const std::string &dir_path) {
	BinarySerializer serializer;
	serializer.serialize_str(dir_path);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::OpenDir, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

int32_t Commander::closedir(int32_t fd) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::CloseDir, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	int32_t value = deserializer.deserialize_int32();
	if (value < 0) {
		_last_errno = deserializer.deserialize_int32();
	}
	return value;
}

std::vector<DirEntry> Commander::readdir(int32_t fd, uint32_t entries) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(entries);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::ReadDir, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	std::vector<DirEntry> dir_entries;
	while (deserializer.bytes_available() > sizeof(int32_t)) {
		auto inode = deserializer.deserialize_uint32();
		auto file_type = deserializer.deserialize_uint8();
		dir_entries.push_back(DirEntry(
				inode, file_type, deserializer.deserialize_str()));
	}
	_last_errno = deserializer.deserialize_int32();
	return dir_entries;
}
