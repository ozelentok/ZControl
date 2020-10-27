#pragma once
#include "TcpSocket.hpp"
#include <string>
#include <vector>
#include <stdint.h>

enum CommanderMessageType: std::uint8_t
{
	Disconnect = 0,
	Open,
	Close,
	Read,
	Write,
};

enum WorkerMessageType: std::uint8_t
{
	CommandResult = 0,
};

class Message {
	public:
		const uint32_t id;
		const uint8_t type;
		const std::vector<uint8_t> data;

		//TODO: Remove copy of data
		Message(uint32_t id, uint8_t type, const std::vector<uint8_t>& data);
};

class MessageTransport {
	private:
		TcpSocket &_socket;
		void _read_exactly(uint8_t* buffer, uint32_t count);

	public:
		MessageTransport(TcpSocket &socket);
		MessageTransport(const MessageTransport&) = delete;
		MessageTransport(MessageTransport&&) = delete;
		~MessageTransport() = default;
		Message read();
		void write(const Message &message);
		void close();
};
