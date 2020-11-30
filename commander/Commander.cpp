#include "Commander.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <stdexcept>
#include <cstring>
#include <sys/stat.h>

Commander::Commander(TcpSocket &&connection)
	: _transport(std::move(connection)), _next_command_id(0),
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

bool Commander::is_connected() const {
	return _connected;
}

void Commander::_read_responses() {
	while (_connected) {
		try {
			auto worker_msg = _transport.read();
			std::lock_guard<std::mutex> lock(_promises_mx);
			auto resp_promise = std::move(_responses_promises.at(worker_msg.id));
			if (worker_msg.type == WorkerMessageType::CommandResult) {
				resp_promise.set_value(worker_msg);
			} else {
				_set_response_exception(resp_promise, worker_msg);
			}
			_responses_promises.erase(worker_msg.id);
		} catch (...) {
			_connected = false;
			std::lock_guard<std::mutex> lock(_promises_mx);
			for (auto it = _responses_promises.begin(); it != _responses_promises.end(); ++it) {
				it->second.set_exception(std::current_exception());
			}
		}
	}
}

void Commander::_set_response_exception(std::promise<Message> &response_promise, const Message &worker_msg) {
	try {
		if (worker_msg.type == WorkerMessageType::CommandError) {
			BinaryDeserializer deserializer(worker_msg.data);
			throw RemoteWorkerException(deserializer.deserialize_str());
		} else if (worker_msg.type == WorkerMessageType::CommandUnknown) {
			throw RemoteWorkerException("Unknown command");
		} else {
			throw std::runtime_error("Unknown response message type");
		}
	} catch (...) {
		response_promise.set_exception(std::current_exception());
	}
}

Message Commander::_send_command(const Message &commander_msg) {
	std::promise<Message> promise;
	auto future = promise.get_future();
	{
		std::lock_guard<std::mutex> lock(_promises_mx);
		if (!_connected) {
			throw RemoteWorkerException("Disconnected");
		}
		_responses_promises[commander_msg.id] = std::move(promise);
	}
	_transport.write(commander_msg);
	Message worker_msg = future.get();
	return worker_msg;
}

void Commander::disconnect() {
	if (!_connected) {
		return;
	}
	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Disconnect, std::vector<uint8_t>(0));
	_send_command(commander_msg);
	_connected = false;
}

std::pair<bool, int32_t> Commander::getattr(const std::string &file_path, struct stat &file_info) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::GetAttr, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	if (value) {
		file_info.st_mode = deserializer.deserialize_uint32();
		file_info.st_uid = deserializer.deserialize_uint32();
		file_info.st_gid = deserializer.deserialize_uint32();
		file_info.st_size = deserializer.deserialize_int64();
		file_info.st_atim.tv_sec = deserializer.deserialize_int64();
		file_info.st_mtim.tv_sec = deserializer.deserialize_int64();
		file_info.st_ctim.tv_sec = deserializer.deserialize_int64();
	}
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::access(const std::string &file_path, int32_t mode) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_int32(mode);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Access, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::rename(const std::string &old_path, const std::string &new_path, uint8_t flags) {
	BinarySerializer serializer;
	serializer.serialize_str(old_path);
	serializer.serialize_str(new_path);
	serializer.serialize_uint8(flags);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Rename, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::truncate(const std::string &file_path, int64_t size) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_int64(size);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Truncate, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::unlink(const std::string &file_path) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Unlink, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::chmod(const std::string &file_path, int32_t mode) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_int32(mode);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::ChangeMode, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::chown(const std::string &file_path, int32_t owner, int32_t group) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_int32(owner);
	serializer.serialize_int32(group);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::ChangeOwner, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::utimens(const std::string &file_path, const struct timespec times[2]) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_int64(times[0].tv_sec);
	serializer.serialize_int64(times[1].tv_sec);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::SetTimes, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::statvfs(const std::string &file_path, struct statvfs *fs_info) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::StatFileSystem, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	fs_info->f_bsize = deserializer.deserialize_uint32();
	fs_info->f_frsize = deserializer.deserialize_uint32();
	fs_info->f_blocks = deserializer.deserialize_uint32();
	fs_info->f_bfree = deserializer.deserialize_uint32();
	fs_info->f_bavail = deserializer.deserialize_uint32();
	fs_info->f_files = deserializer.deserialize_uint32();
	fs_info->f_ffree = deserializer.deserialize_uint32();
	fs_info->f_favail = deserializer.deserialize_uint32();
	fs_info->f_fsid = deserializer.deserialize_uint32();
	fs_info->f_flag = deserializer.deserialize_uint32();
	fs_info->f_namemax = deserializer.deserialize_uint32();
	return std::make_pair(value, worker_errno);
}

std::pair<int32_t, int32_t> Commander::open(const std::string &file_path, int32_t flags, int32_t mode) {
	BinarySerializer serializer;
	serializer.serialize_str(file_path);
	serializer.serialize_int32(flags);
	serializer.serialize_int32(mode);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Open, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_int32();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::close(int32_t fd) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Close, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<int32_t, int32_t> Commander::read(int32_t fd, uint8_t *bytes, uint32_t size) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(size);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::Read, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_int32();
	auto worker_errno = deserializer.deserialize_int32();
	if (value > 0) {
		auto bytes_vector = deserializer.deserialize_vector();
		if (bytes_vector.size() > size) {
			throw std::runtime_error("read returned more bytes than requested");
		}
		::memcpy(bytes, bytes_vector.data(), value);
	}
	return std::make_pair(value, worker_errno);
}

std::pair<int32_t, int32_t> Commander::pread(int32_t fd, uint8_t *bytes, uint32_t size, int64_t offset) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);
	serializer.serialize_uint32(size);
	serializer.serialize_int64(offset);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::PRead, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_int32();
	auto worker_errno = deserializer.deserialize_int32();
	if (value > 0) {
		auto bytes_vector = deserializer.deserialize_vector();
		if (bytes_vector.size() > size) {
			throw std::runtime_error("pread returned more bytes than requested");
		}
		::memcpy(bytes, bytes_vector.data(), value);
	}
	return std::make_pair(value, worker_errno);
}

std::pair<int32_t, int32_t> Commander::write(int32_t fd, const uint8_t *bytes, uint32_t size) {
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

	auto value = deserializer.deserialize_int32();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<int32_t, int32_t> Commander::pwrite(int32_t fd, const uint8_t *bytes, uint32_t size, int64_t offset) {
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

	auto value = deserializer.deserialize_int32();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<int32_t, int32_t> Commander::opendir(const std::string &dir_path) {
	BinarySerializer serializer;
	serializer.serialize_str(dir_path);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::OpenDir, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_int32();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::closedir(int32_t fd) {
	BinarySerializer serializer;
	serializer.serialize_int32(fd);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::CloseDir, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<std::vector<DirEntry>, int32_t> Commander::readdir(int32_t fd, uint32_t entries) {
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
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(dir_entries, worker_errno);
}

std::pair<bool, int32_t> Commander::mkdir(const std::string &dir_path, int32_t mode) {
	BinarySerializer serializer;
	serializer.serialize_str(dir_path);
	serializer.serialize_int32(mode);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::MakeDir, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}

std::pair<bool, int32_t> Commander::rmdir(const std::string &dir_path) {
	BinarySerializer serializer;
	serializer.serialize_str(dir_path);

	auto commander_msg = Message(_next_command_id++, CommanderMessageType::RemoveDir, serializer.data());
	Message worker_msg = _send_command(commander_msg);
	BinaryDeserializer deserializer(worker_msg.data);

	auto value = deserializer.deserialize_uint8();
	auto worker_errno = deserializer.deserialize_int32();
	return std::make_pair(value, worker_errno);
}
