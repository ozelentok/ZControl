#pragma once
#include "TcpSocket.hpp"
#include "Commander.hpp"
#include <map>
#include <thread>
#include <atomic>

class Server {
private:
  TcpSocket _server;
  std::map<std::string, std::shared_ptr<Commander>> _commanders;
  std::mutex _commanders_mx;
  std::thread _acceptor_thread;
  std::atomic_bool _should_stop;
  void _accept_connections();

public:
  Server(const std::string &host, uint16_t port);
  Server(const Server &other) = delete;
  Server(Server &&other) = delete;
  Server &operator=(const Server &other) = delete;
  Server &operator=(Server &&other) = delete;
  ~Server();
  void start();
  void stop();
  std::shared_ptr<Commander> get_commander(const std::string &client);
  std::vector<std::string> get_clients();
};
