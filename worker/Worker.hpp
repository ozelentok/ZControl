#pragma once
#include "ConcurrentQueue.hpp"
#include "DirCommandsHandler.hpp"
#include "FileCommandsHandler.hpp"
#include "MessageTransport.hpp"
#include "ThreadPool.hpp"

class Worker {
private:
  MessageTransport _transport;
  ThreadPool _thread_pool;
  ConcurrentQueue<Message> _message_queue;
  FileCommandsHandler _file_handler;
  DirCommandsHandler _dir_handler;
  bool _should_stop;

  void __handle_messages();
  Message _disconnect(const Message &message);
  Message _do_command(const Message &commander_msg);

public:
  Worker(const std::string &host, uint16_t port);
  Worker(const Worker &other) = delete;
  Worker(Worker &&other) = delete;
  Worker &operator=(const Worker &other) = delete;
  Worker &operator=(Worker &&other) = delete;
  ~Worker();
  void work();
  void stop();
};
