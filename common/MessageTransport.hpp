#pragma once
#include "TcpSocket.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <stdint.h>

enum CommanderMessageType: std::uint8_t
{
	Disconnect = 0,
	GetAttr,
	Access,
	Open,
	Close,
	Read,
	PRead,
	Write,
	PWrite,
	OpenDir,
	CloseDir,
	ReadDir,
};

enum WorkerMessageType: std::uint8_t
{
	CommandResult = 0,
};

class Message {
	public:
		uint32_t id;
		uint8_t type;
		std::vector<uint8_t> data;

		Message(uint32_t id, uint8_t type, const std::vector<uint8_t>&& data);
};


class TransportClosed: public std::runtime_error {
	public:
		TransportClosed(): std::runtime_error("Transport Closed") {}
};

class MessageTransport {
	private:
		TcpSocket _socket;
		std::mutex _read_mx;
		std::mutex _write_mx;
		void _read_exactly(uint8_t* buffer, uint32_t count);

	public:
		MessageTransport(const std::string &host, uint16_t port);
		MessageTransport(TcpSocket &&socket);
		MessageTransport(const MessageTransport&) = delete;
		MessageTransport(MessageTransport&&) = delete;
		~MessageTransport() = default;
		Message read();
		void write(const Message &message);
		void close();
};
