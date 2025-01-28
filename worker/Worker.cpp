#include "Worker.hpp"
#include "BinarySerializer.hpp"
#include "SysLog.hpp"
#include <functional>

Worker::Worker(const std::string &host, uint16_t port)
    : _transport(host, port), _thread_pool(std::thread::hardware_concurrency()), _should_stop(false) {
  _thread_pool.submit(std::bind(&Worker::__handle_messages, this));
}

void Worker::work() {
  while (!_should_stop) {
    try {
      Message commander_msg(_transport.read());
      SYSLOG_DEBUG("Got message: id: %d, type: %d, data_length: %zu", commander_msg.id, commander_msg.type,
                   commander_msg.data.size());
      _message_queue.push(std::move(commander_msg));
    } catch (const TransportClosed &) {
      _should_stop = true;
    } catch (const CancellationException &) {
      _should_stop = true;
    } catch (const std::exception &e) {
      _should_stop = true;
      SYSLOG_ERROR("Error reading message from server: %s", e.what());
    } catch (...) {
      _should_stop = true;
      SYSLOG_ERROR("Unknwon Error reading message from server");
    }
  }
}

void Worker::stop() {
  _should_stop = true;
  _transport.close();
}

void Worker::__handle_messages() {
  while (!_should_stop) {
    try {
      const Message msg = _message_queue.pop();
      _transport.write(_do_command(msg));
    } catch (const QueueShutdown &) {
      _should_stop = true;
    } catch (const TransportClosed &) {
      _should_stop = true;
      SYSLOG_ERROR("Transport closed while handling message");
    } catch (const std::exception &e) {
      _should_stop = true;
      SYSLOG_ERROR("Error handling message: %s", e.what());
    } catch (...) {
      _should_stop = true;
      SYSLOG_ERROR("Unknwon Error handling message");
    }
  }
}

Message Worker::_disconnect(const Message &message) {
  _should_stop = true;
  return Message(message.id, WorkerMessageType::CommandResult, std::vector<uint8_t>(0));
}

Message Worker::_do_command(const Message &commander_msg) {
  try {
    switch (commander_msg.type) {
    case CommanderMessageType::Disconnect:
      return _disconnect(commander_msg);
    case CommanderMessageType::GetAttr:
      return _file_handler.getattr(commander_msg);
    case CommanderMessageType::Access:
      return _file_handler.access(commander_msg);
    case CommanderMessageType::Rename:
      return _file_handler.rename(commander_msg);
    case CommanderMessageType::Truncate:
      return _file_handler.truncate(commander_msg);
    case CommanderMessageType::Unlink:
      return _file_handler.unlink(commander_msg);
    case CommanderMessageType::ChangeMode:
      return _file_handler.chmod(commander_msg);
    case CommanderMessageType::ChangeOwner:
      return _file_handler.chown(commander_msg);
    case CommanderMessageType::SetTimes:
      return _file_handler.utimens(commander_msg);
    case CommanderMessageType::StatFileSystem:
      return _file_handler.statvfs(commander_msg);
    case CommanderMessageType::Open:
      return _file_handler.open(commander_msg);
    case CommanderMessageType::Close:
      return _file_handler.close(commander_msg);
    case CommanderMessageType::Read:
      return _file_handler.read(commander_msg);
    case CommanderMessageType::PRead:
      return _file_handler.pread(commander_msg);
    case CommanderMessageType::Write:
      return _file_handler.write(commander_msg);
    case CommanderMessageType::PWrite:
      return _file_handler.pwrite(commander_msg);
    case CommanderMessageType::OpenDir:
      return _dir_handler.opendir(commander_msg);
    case CommanderMessageType::CloseDir:
      return _dir_handler.closedir(commander_msg);
    case CommanderMessageType::ReadDir:
      return _dir_handler.readdir(commander_msg);
    case CommanderMessageType::MakeDir:
      return _dir_handler.mkdir(commander_msg);
    case CommanderMessageType::RemoveDir:
      return _dir_handler.rmdir(commander_msg);
    default:
      return Message(commander_msg.id, WorkerMessageType::CommandUnknown, std::vector<uint8_t>(0));
    }
  } catch (const std::exception &e) {
    BinarySerializer serializer;
    serializer.serialize_str(e.what());
    return Message(commander_msg.id, WorkerMessageType::CommandError, serializer.data());
  } catch (...) {
    BinarySerializer serializer;
    serializer.serialize_str("Unknown exception");
    return Message(commander_msg.id, WorkerMessageType::CommandError, serializer.data());
  }
}
