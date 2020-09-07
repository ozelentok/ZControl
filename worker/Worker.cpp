#include "Worker.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

Worker::Worker(const std::string &host, uint16_t port) :
	_connection(), _transport(_connection), _next_fd_id(0) {
		_connection.connect(host, port);
	}

int32_t Worker::_get_fd(int32_t fd_id) const {
	//TODO: what to do if fd_id does not exist?
	return _fds.at(fd_id);
}

void Worker::work() {
	while (true) {
		Message m_in;
		printf("Waiting for message\n");
		_transport.read(m_in);
		printf("Got message, sleeping a bit\n");
		BinaryDeserializer deserializer(m_in.data);
		BinarySerializer serializer;

		uint32_t fd_id = 0;
		int32_t value = 0;

		switch (m_in.type.commander) {
			case CommanderMessageType::Open:
				{
					Message m_out;
					const std::string file_path = deserializer.deserialize_str();
					const int32_t flags = deserializer.deserialize_uint32();
					const int32_t fd = ::open(file_path.c_str(), flags, DEFFILEMODE);
					fd_id = -1;

					if (fd >= 0) {
						fd_id = _next_fd_id++;
						_fds.emplace(fd_id, fd);
					}

					serializer.serialize_int32(fd_id);
					serializer.serialize_int32(errno);
					m_out.type.worker = WorkerMessageType::CommandResult;
					m_out.data = serializer.data();
					_transport.write(m_out);
					break;
				}

			case CommanderMessageType::Close:
				{
					Message m_out;
					fd_id = deserializer.deserialize_uint32();
					value = ::close(_get_fd(fd_id));
					_fds.erase(fd_id);
					serializer.serialize_int32(value);
					serializer.serialize_int32(errno);
					m_out.type.worker = WorkerMessageType::CommandResult;
					m_out.data = serializer.data();
					_transport.write(m_out);
					break;
				}

			case CommanderMessageType::Read:
				{
					Message m_out;
					fd_id = deserializer.deserialize_uint32();
					uint32_t size = deserializer.deserialize_uint32();
					std::vector<uint8_t> buf(size);
					value = ::read(_get_fd(fd_id), buf.data(), size);
					serializer.serialize_int32(value);
					serializer.serialize_int32(errno);
					if (value >= 0) {
						buf.resize(value);
					}
					serializer.serialize_vector(buf);
					m_out.type.worker = WorkerMessageType::CommandResult;
					m_out.data = serializer.data();
					_transport.write(m_out);
					break;
				}

			case CommanderMessageType::Write:
				{
					Message m_out;
					fd_id = deserializer.deserialize_uint32();
					auto bytes = deserializer.deserialize_vector();
					value = ::write(_get_fd(fd_id), bytes.data(), bytes.size());
					serializer.serialize_int32(value);
					serializer.serialize_int32(errno);
					m_out.type.worker = WorkerMessageType::CommandResult;
					m_out.data = serializer.data();
					_transport.write(m_out);
					break;
				}
		}
	}
}
