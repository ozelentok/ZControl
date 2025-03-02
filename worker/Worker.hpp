#pragma once
#include "ConcurrentQueue.hpp"
#include "DirCommandsHandler.hpp"
#include "FileCommandsHandler.hpp"
#include "MessageTransport.hpp"
#include "ThreadPool.hpp"
#include <thread>

class Worker {
private:
  bool _should_stop;
  FileCommandsHandler _file_handler;
  DirCommandsHandler _dir_handler;
  MessageTransport _transport;
  ConcurrentQueue<Message> _message_queue;
  ThreadPool _handlers_pool;
  std::thread _messages_reader;

  void _read_messages();
  void _handle_messages();
  Message _disconnect(const Message &message);
  Message _do_command(const Message &commander_msg);

public:
  Worker(const std::string &host, uint16_t port);
  Worker(const Worker &other) = delete;
  Worker(Worker &&other) = delete;
  Worker &operator=(const Worker &other) = delete;
  Worker &operator=(Worker &&other) = delete;
  ~Worker();
  void close();
  void wait();
};
