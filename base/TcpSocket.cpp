#include "TcpSocket.hpp"
#include "AddressInfo.hpp"
#include "Logger.hpp"
#include <system_error>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <poll.h>

static const int NoTimeout = -1;

TcpSocket::TcpSocket() {
  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket == -1) {
    throw std::system_error(errno, std::system_category(), "Failed to create socket");
  }
}

TcpSocket::TcpSocket(int socket) : _socket(socket) {}

TcpSocket::TcpSocket(TcpSocket &&other) : _socket(-1) {
  std::swap(_socket, other._socket);
}

TcpSocket::~TcpSocket() {
  DTOR_TRY
  close();
  DTOR_CATCH
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

  int enable = 1;
  int result = setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));
  if (result == -1) {
    throw std::system_error(errno, std::system_category(), "Failed to set socket option REUSEADDR");
  }

  result = ::bind(_socket, addr.get()->ai_addr, addr.get()->ai_addrlen);
  if (result == -1) {
    throw std::system_error(errno, std::system_category(), "Failed to bind");
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
    ::shutdown(_socket, SHUT_RDWR);
    ::close(_socket);
    _socket = -1;
  }
}

std::tuple<TcpSocket, uint32_t, uint16_t> TcpSocket::accept() {
  struct sockaddr_in client_address;
  socklen_t client_address_size = sizeof(client_address);
  int socket = ::accept(_socket, reinterpret_cast<struct sockaddr *>(&client_address), &client_address_size);
  if (socket == -1) {
    throw std::system_error(errno, std::system_category(), "Failed to accept");
  }
  return std::make_tuple(TcpSocket(socket), client_address.sin_addr.s_addr, client_address.sin_port);
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
  ssize_t result = ::send(_socket, bytes, size, MSG_NOSIGNAL);
  if (result <= -1) {
    throw std::system_error(errno, std::system_category(), "Failed to send");
  }
}

bool TcpSocket::poll(int cancellation_pipe) {
  pollfd pfds[] = {{.fd = _socket, .events = POLLIN, .revents = 0},
                   {.fd = cancellation_pipe, .events = POLLIN, .revents = 0}};
  int result = ::poll(pfds, sizeof(pfds) / sizeof(*pfds), NoTimeout);
  if (result <= -1) {
    throw std::system_error(errno, std::system_category(), "Failed to poll");
  }
  if (pfds[1].revents & POLLIN) {
    uint8_t buf[64];
    ::read(cancellation_pipe, buf, sizeof(buf) / sizeof(*buf));
    return false;
  }
  return true;
}

std::string TcpSocket::format_connection(uint32_t ip, uint16_t port) {
  struct in_addr addr;
  addr.s_addr = ip;
  return std::string(inet_ntoa(addr));
}
