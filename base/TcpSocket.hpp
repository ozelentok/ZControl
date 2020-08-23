#pragma once
#include <string>
#include <vector>
#include <stdint.h>

class TcpSocket {
	private:
		int _socket;

	public:
		TcpSocket();
		TcpSocket(int socket);
		TcpSocket(const TcpSocket&) = delete;
		~TcpSocket();
		void connect(const std::string &host, uint16_t port);
		void send(const std::vector<uint8_t> bytes);
		void send(const uint8_t *bytes, size_t size);
		size_t recv(uint8_t *bytes, size_t size);
};
