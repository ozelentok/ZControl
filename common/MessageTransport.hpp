#pragma once
#include "TcpSocket.hpp"
#include "CancellationPipe.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <stdint.h>

enum CommanderMessageType: std::uint8_t
{
	Disconnect = 0,
	GetAttr,
	Access,
	Rename,
	Truncate,
	Unlink,
	ChangeMode,
	ChangeOwner,
	SetTimes,
	StatFileSystem,
	Open,
	Close,
	Read,
	PRead,
	Write,
	PWrite,
	OpenDir,
	CloseDir,
	ReadDir,
	MakeDir,
	RemoveDir,
	GetXAttr,
	SetXAttr,
	ListXAttr,
	RemoveXAttr,
};

enum WorkerMessageType: std::uint8_t
{
	CommandResult = 0,
	CommandError,
	CommandUnknown,
};

class Message {
	public:
		uint32_t id;
		uint8_t type;
		std::vector<uint8_t> data;

		Message(uint32_t id, uint8_t type, std::vector<uint8_t> &&data);
};

class TransportClosed: public std::runtime_error {
	public:
		TransportClosed(): std::runtime_error("Transport Closed") {}
};

class MessageTransport {
	private:
		TcpSocket _socket;
		CancellationPipe _poll_canceling_pipe;
		std::mutex _read_mx;
		std::mutex _write_mx;
		void _read_exactly(uint8_t *buffer, uint32_t count);

	public:
		MessageTransport(const std::string &host, uint16_t port);
		MessageTransport(TcpSocket &&socket);
		MessageTransport(const MessageTransport &other) = delete;
		MessageTransport(MessageTransport &&other) = delete;
		MessageTransport& operator=(const MessageTransport &other) = delete;
		MessageTransport& operator=(MessageTransport &&other) = delete;
		~MessageTransport() = default;
		Message read();
		void write(const Message &message);
		void close();
};
