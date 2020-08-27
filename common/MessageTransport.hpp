#pragma once
#include "TcpSocket.hpp"
#include <string>
#include <vector>
#include <stdint.h>

enum CommanderMessageType: std::uint8_t
{
	Open = 0,
	Close,
	Read,
	Write,
};


#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
// TODO: Remove length from logic code
struct Message {
	uint32_t id;
	CommanderMessageType type;
	uint32_t length;
	std::vector<uint8_t> data;
};
#pragma pack(pop)   /* restore original alignment from stack */


class MessageTransport {
	private:
		TcpSocket &_socket;

	void _read_exactly(uint8_t* buffer, uint32_t count);
	public:
		MessageTransport(TcpSocket &socket);
		MessageTransport(const MessageTransport&) = delete;
		MessageTransport(MessageTransport&&) = delete;
		~MessageTransport() = default;
		void read(Message &message);
		void write(const Message &message);
};
