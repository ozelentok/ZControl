#include "Worker.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <functional>


Worker::Worker(const std::string &host, uint16_t port) :
	_transport(host, port),
	_thread_pool(std::thread::hardware_concurrency()),
	_should_disconnect(false) {
}

void Worker::work() {
	while (!_should_disconnect) {
		try {
			Message commander_msg(_transport.read());
			printf("Got message: id: %d, type: %d, data_length: %ld\n",
						 commander_msg.id, commander_msg.type, commander_msg.data.size());
			_thread_pool.submit(std::bind(&Worker::_handle_commander_message, this, std::move(commander_msg)));
		} catch (const TransportClosed&) {
			_should_disconnect = true;
		} catch (...) {
			_should_disconnect = true;
			printf("Unknown exception\n");
		}
	}
}

void Worker::_handle_commander_message(const Message &commander_msg) {
	switch (commander_msg.type) {
		case CommanderMessageType::Disconnect:
			_transport.write(_disconnect(commander_msg));
			break;
		case CommanderMessageType::GetAttr:
			_transport.write(_file_handler.getattr(commander_msg));
			break;
		case CommanderMessageType::Access:
			_transport.write(_file_handler.access(commander_msg));
			break;
		case CommanderMessageType::Rename:
			_transport.write(_file_handler.rename(commander_msg));
			break;
		case CommanderMessageType::Truncate:
			_transport.write(_file_handler.truncate(commander_msg));
			break;
		case CommanderMessageType::Unlink:
			_transport.write(_file_handler.unlink(commander_msg));
			break;
		case CommanderMessageType::ChangeMode:
			_transport.write(_file_handler.chmod(commander_msg));
			break;
		case CommanderMessageType::ChangeOwner:
			_transport.write(_file_handler.chown(commander_msg));
			break;
		case CommanderMessageType::SetTimes:
			_transport.write(_file_handler.utimens(commander_msg));
			break;
		case CommanderMessageType::StatFileSystem:
			_transport.write(_file_handler.statvfs(commander_msg));
			break;
		case CommanderMessageType::Open:
			_transport.write(_file_handler.open(commander_msg));
			break;
		case CommanderMessageType::Close:
			_transport.write(_file_handler.close(commander_msg));
			break;
		case CommanderMessageType::Read:
			_transport.write(_file_handler.read(commander_msg));
			break;
		case CommanderMessageType::PRead:
			_transport.write(_file_handler.pread(commander_msg));
			break;
		case CommanderMessageType::Write:
			_transport.write(_file_handler.write(commander_msg));
			break;
		case CommanderMessageType::PWrite:
			_transport.write(_file_handler.pwrite(commander_msg));
			break;
		case CommanderMessageType::OpenDir:
			_transport.write(_dir_handler.opendir(commander_msg));
			break;
		case CommanderMessageType::CloseDir:
			_transport.write(_dir_handler.closedir(commander_msg));
			break;
		case CommanderMessageType::ReadDir:
			_transport.write(_dir_handler.readdir(commander_msg));
			break;
		case CommanderMessageType::MakeDir:
			_transport.write(_dir_handler.mkdir(commander_msg));
			break;
		case CommanderMessageType::RemoveDir:
			_transport.write(_dir_handler.rmdir(commander_msg));
			break;
		//TODO: Unknown command, respond somehow
		default:
			break;
	}
}

Message Worker::_disconnect(const Message& message) {
	_should_disconnect = true;
	return Message(message.id, WorkerMessageType::CommandResult, std::vector<uint8_t>(0));
}
