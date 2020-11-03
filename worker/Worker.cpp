#include "Worker.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <functional>


Worker::Worker(const std::string &host, uint16_t port) :
	_connection(), _transport(_connection),
	_thread_pool(std::thread::hardware_concurrency()),
	_should_disconnect(false) {
	_connection.connect(host, port);
}


void Worker::work() {
	while (!_should_disconnect) {
		try {
			Message commander_msg(_transport.read());
			printf("Got message: id: %d, type: %d, data_length: %ld\n",
						 commander_msg.id, commander_msg.type, commander_msg.data.size());
			_thread_pool.submit(std::bind(&Worker::_handle_commander_message, this, std::move(commander_msg)));
		} catch (const TransportClosed) {
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
		case CommanderMessageType::Open:
			_transport.write(_file_handler.open(commander_msg));
			break;
		case CommanderMessageType::Close:
			_transport.write(_file_handler.close(commander_msg));
			break;
		case CommanderMessageType::Read:
			_transport.write(_file_handler.read(commander_msg));
			break;
		case CommanderMessageType::Write:
			_transport.write(_file_handler.write(commander_msg));
			break;
	}
}

Message Worker::_disconnect(const Message& message) {
	_should_disconnect = true;
	printf("got disconnect message\n");
	return Message(message.id, WorkerMessageType::CommandResult, std::vector<uint8_t>(0));
}
