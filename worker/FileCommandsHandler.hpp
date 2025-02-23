#pragma once
#include "MessageTransport.hpp"
#include <map>
#include <mutex>
#include <atomic>

class FileCommandsHandler {
private:
  std::map<int32_t, int32_t> _fds;
  std::mutex _fds_mx;
  std::atomic_int32_t _next_fd_id;
  int32_t _get_fd(int32_t fd_id);

public:
  FileCommandsHandler();
  FileCommandsHandler(const FileCommandsHandler &other) = delete;
  FileCommandsHandler(FileCommandsHandler &&other) = delete;
  FileCommandsHandler &operator=(const FileCommandsHandler &other) = delete;
  FileCommandsHandler &operator=(FileCommandsHandler &&other) = delete;
  ~FileCommandsHandler();
  Message getattr(const Message &message);
  Message access(const Message &message);
  Message rename(const Message &message);
  Message truncate(const Message &message);
  Message unlink(const Message &message);
  Message chmod(const Message &message);
  Message chown(const Message &message);
  Message utimens(const Message &message);
  Message statvfs(const Message &message);
  Message open(const Message &message);
  Message close(const Message &message);
  Message read(const Message &message);
  Message pread(const Message &message);
  Message write(const Message &message);
  Message pwrite(const Message &message);
  Message getxattr(const Message &message);
  Message setxattr(const Message &message);
  Message listxattr(const Message &message);
  Message removexattr(const Message &message);
};
