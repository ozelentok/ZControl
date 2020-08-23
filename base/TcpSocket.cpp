#include "TcpSocket.hpp"
#include "AddressInfo.hpp"
#include <system_error>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

TcpSocket::TcpSocket() {
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to create socket");
	}
}

TcpSocket::TcpSocket(int socket) : _socket(socket) {}

TcpSocket::~TcpSocket() {
	try {
		if (_socket != -1) {
			close(_socket);
			_socket = -1;
		}
	} catch (...) {}
}

void TcpSocket::connect(const std::string &host, uint16_t port) {
	AddressInfo addr(host, port);
	if (addr.is_empty()) {
		throw	std::runtime_error("Unable to resolve hostname");
	}
	int result = ::connect(this->_socket, addr.get()->ai_addr, addr.get()->ai_addrlen);
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to connect");
	}
}

void TcpSocket::send(const std::vector<uint8_t> bytes) {
	this->send(bytes.data(), bytes.size());
}

void TcpSocket::send(const uint8_t *bytes, size_t size) {
	ssize_t result = ::send(this->_socket, bytes, size, 0);
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to write");
	}
}

size_t TcpSocket::recv(uint8_t *bytes, size_t size) {
	ssize_t result = ::recv(this->_socket, bytes, size, 0);
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to recv");
	}
	return result;
}
