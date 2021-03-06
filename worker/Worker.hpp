#pragma once
#include "TcpSocket.hpp"
#include "MessageTransport.hpp"
#include "ThreadPool.hpp"
#include "FileCommandsHandler.hpp"
#include "DirCommandsHandler.hpp"

class Worker {
private:
  MessageTransport _transport;
  ThreadPool _thread_pool;
  FileCommandsHandler _file_handler;
  DirCommandsHandler _dir_handler;
  bool _should_stop;

  Message _disconnect(const Message &message);
  void _handle_commander_message(const Message &commander_msg);
  Message _do_command(const Message &commander_msg);

public:
  Worker(const std::string &host, uint16_t port);
  Worker(const Worker &other) = delete;
  Worker(Worker &&other) = delete;
  Worker &operator=(const Worker &other) = delete;
  Worker &operator=(Worker &&other) = delete;
  ~Worker() = default;
  void work();
  void stop();
};
