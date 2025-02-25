#pragma once
#include "ConcurrentQueue.hpp"
#include "DirCommandsHandler.hpp"
#include "FileCommandsHandler.hpp"
#include "MessageTransport.hpp"
#include "ThreadPool.hpp"
#include <thread>

class Worker {
private:
  MessageTransport _transport;
  std::thread _reader_thread;
  ThreadPool _handlers_pool;
  ConcurrentQueue<Message> _message_queue;
  FileCommandsHandler _file_handler;
  DirCommandsHandler _dir_handler;
  bool _should_stop;

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
