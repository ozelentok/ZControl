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

TcpSocket::TcpSocket(TcpSocket&& other) : _socket(-1) {
	std::swap(_socket, other._socket);
}

TcpSocket::~TcpSocket() {
	try {
		close();
	} catch (...) {}
}

void TcpSocket::connect(const std::string &host, uint16_t port) {
	AddressInfo addr(host, port);
	if (addr.is_empty()) {
		throw std::runtime_error("Unable to resolve hostname: " + host);
	}
	int result = ::connect(_socket, addr.get()->ai_addr, addr.get()->ai_addrlen);
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to connect");
	}
}

void TcpSocket::bind(const std::string &host, uint16_t port) {
	AddressInfo addr(host, port);
	if (addr.is_empty()) {
		throw std::runtime_error("Unable to resolve hostname: " + host);
	}
	int result = ::bind(_socket, addr.get()->ai_addr, addr.get()->ai_addrlen);
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to bind");
	}

	int enable = 1;
	result = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to set socket option REUSEADDR");
	}
	result = setsockopt(_socket, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable));
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to set socket option REUSEPORT");
	}
}

void TcpSocket::listen(int backlog) {
	int result = ::listen(_socket, backlog);
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to set backlog");
	}
}

void TcpSocket::close() {
	if (_socket != -1) {
		::close(_socket);
		_socket = -1;
	}
}

TcpSocket TcpSocket::accept() {
	int socket = ::accept(_socket, nullptr, nullptr);
	if (socket == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to accept");
	}
	return TcpSocket(socket);
}

size_t TcpSocket::recv(uint8_t *bytes, size_t size) {
	ssize_t result = ::recv(_socket, bytes, size, 0);
	if (result <= -1) {
		throw std::system_error(errno, std::system_category(), "Failed to recv");
	}
	return result;
}

void TcpSocket::send(const std::vector<uint8_t> bytes) {
	send(bytes.data(), bytes.size());
}

void TcpSocket::send(const uint8_t *bytes, size_t size) {
	ssize_t result = ::send(_socket, bytes, size, 0);
	if (result <= -1) {
		throw std::system_error(errno, std::system_category(), "Failed to send");
	}
}
