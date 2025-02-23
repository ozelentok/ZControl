#include "DirCommandsHandler.hpp"
#include "BinarySerializer.hpp"
#include "BinaryDeserializer.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>

#ifdef ANDROID
const std::map<std::string, const std::vector<dirent>> DirCommandsHandler::DirsToOverride = {
    {"/", {{.d_ino = 0, .d_type = DT_DIR, .d_name = "sdcard"}, {.d_ino = 0, .d_type = DT_DIR, .d_name = "storage"}}},
    {"/storage", {{.d_ino = 0, .d_type = DT_DIR, .d_name = "emulated"}}},
    {"/storage/emulated", {{.d_ino = 0, .d_type = DT_DIR, .d_name = "0"}}}};
#else
const std::map<std::string, const std::vector<dirent>> DirCommandsHandler::DirsToOverride;
#endif

DirCommandsHandler::DirCommandsHandler() : _next_fd_id(0) {}

DirCommandsHandler::~DirCommandsHandler() {
  try {
    std::lock_guard<std::mutex> lock(_fds_mx);
    for (auto const &it : _fds) {
      ::closedir(it.second);
    }
  } catch (...) {
  }
}

DIR *DirCommandsHandler::_get_fd(int32_t fd_id) {
  std::lock_guard<std::mutex> lock(_fds_mx);
  auto iter = _fds.find(fd_id);
  if (iter == _fds.end()) {
    syslog(LOG_WARNING, "Server used an invalid directory descriptor id: %d", fd_id);
    return nullptr;
  }
  return iter->second;
}

Message DirCommandsHandler::opendir(const Message &message) {
  BinaryDeserializer deserializer(message.data);
  const auto dir_path = deserializer.deserialize_str();

  auto overridden_dir = DirsToOverride.find(dir_path);
  bool should_override = overridden_dir != DirsToOverride.end();
  int32_t fd_id = -1;

  if (should_override) {
    fd_id = _next_fd_id++;
    std::lock_guard<std::mutex> lock(_fds_mx);
    _overridden_fds.emplace(fd_id, &(overridden_dir->second));
  } else {
    DIR *fd = ::opendir(dir_path.c_str());
    if (fd != nullptr) {
      fd_id = _next_fd_id++;
      std::lock_guard<std::mutex> lock(_fds_mx);
      _fds.emplace(fd_id, fd);
    }
  }

  BinarySerializer serializer;
  serializer.serialize_int32(fd_id);
  serializer.serialize_int32(should_override ? 0 : errno);
  return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message DirCommandsHandler::closedir(const Message &message) {
  BinaryDeserializer deserializer(message.data);
  const auto fd_id = deserializer.deserialize_int32();

  auto overridden_iter = _overridden_fds.find(fd_id);
  auto should_override = overridden_iter != _overridden_fds.end();
  int32_t value = 0;

  if (should_override) {
    if (overridden_iter != _overridden_fds.end()) {
      std::lock_guard<std::mutex> lock(_fds_mx);
      _overridden_fds.erase(overridden_iter);
    }
  } else {
    value = ::closedir(_get_fd(fd_id));
    std::lock_guard<std::mutex> lock(_fds_mx);
    _fds.erase(fd_id);
  }

  BinarySerializer serializer;
  serializer.serialize_uint8(value == 0 ? 1 : 0);
  serializer.serialize_int32(should_override ? 0 : errno);
  return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message DirCommandsHandler::readdir(const Message &message) {
  BinaryDeserializer deserializer(message.data);
  const auto fd_id = deserializer.deserialize_int32();
  const auto entries = deserializer.deserialize_uint32();

  auto overridden_iter = _overridden_fds.find(fd_id);
  auto should_override = overridden_iter != _overridden_fds.end();
  DIR *fd = 0;

  BinarySerializer serializer;
  if (!should_override) {
    fd = _get_fd(fd_id);
    if (fd == nullptr) {
      serializer.serialize_int32(EBADF);
      return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
    }
  }

  for (uint32_t i = 0; i < entries; i++) {
    const dirent *dir = nullptr;
    if (!should_override) {
      errno = 0;
      dir = ::readdir(fd);
    } else if (i < overridden_iter->second->size()) {
      dir = &overridden_iter->second->at(i);
    }

    if (dir == nullptr) {
      break;
    }
    serializer.serialize_uint32(dir->d_ino);
    serializer.serialize_uint8(dir->d_type);
    serializer.serialize_str(dir->d_name);
  }
  serializer.serialize_int32(should_override ? 0 : errno);
  return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message DirCommandsHandler::mkdir(const Message &message) {
  BinaryDeserializer deserializer(message.data);
  const auto dir_path = deserializer.deserialize_str();
  const auto mode = deserializer.deserialize_int32();

  const int32_t value = ::mkdir(dir_path.c_str(), mode);

  BinarySerializer serializer;
  serializer.serialize_uint8(value == 0 ? 1 : 0);
  serializer.serialize_int32(errno);
  return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}

Message DirCommandsHandler::rmdir(const Message &message) {
  BinaryDeserializer deserializer(message.data);
  const auto dir_path = deserializer.deserialize_str();

  const int32_t value = ::rmdir(dir_path.c_str());

  BinarySerializer serializer;
  serializer.serialize_uint8(value == 0 ? 1 : 0);
  serializer.serialize_int32(errno);
  return Message(message.id, WorkerMessageType::CommandResult, serializer.data());
}
