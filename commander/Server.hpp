#pragma once
#include "TcpSocket.hpp"
#include "Commander.hpp"
#include <map>
#include <thread>
#include <atomic>

class Server {
private:
  std::atomic_bool _should_stop;
  std::mutex _commanders_mx;
  std::map<std::string, std::shared_ptr<Commander>> _commanders;
  TcpSocket _server;
  std::jthread _acceptor_thread;
  void _accept_connections();

public:
  Server(const std::string &host, uint16_t port);
  Server(const Server &other) = delete;
  Server(Server &&other) = delete;
  Server &operator=(const Server &other) = delete;
  Server &operator=(Server &&other) = delete;
  ~Server();
  void close();
  std::shared_ptr<Commander> get_commander(const std::string &client);
  std::vector<std::string> get_clients();
};
