#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <stdint.h>

class TcpSocket {
	private:
		int _socket;

	public:
		TcpSocket();
		TcpSocket(int socket);
		TcpSocket(const TcpSocket &other) = delete;
		TcpSocket(TcpSocket &&other);
		TcpSocket& operator=(const TcpSocket &other) = delete;
		TcpSocket& operator=(TcpSocket &&other) = delete;
		~TcpSocket();
		void connect(const std::string &host, uint16_t port);
		void bind(const std::string &host, uint16_t port);
		void listen(int backlog);
		void close();
		std::tuple<TcpSocket, uint32_t, uint16_t> accept();
		size_t recv(uint8_t *bytes, size_t size);
		void send(const std::vector<uint8_t> bytes);
		void send(const uint8_t *bytes, size_t size);
		static std::string format_connection(uint32_t ip, uint16_t port);
};
