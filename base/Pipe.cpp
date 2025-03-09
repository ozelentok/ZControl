#include "Pipe.hpp"
#include "Logger.hpp"
#include <system_error>
#include <errno.h>

Pipe::Pipe() : _pipe_fds{-1, -1} {
  int result = pipe(_pipe_fds);
  if (result != 0) {
    throw std::system_error(errno, std::system_category(), "Failed to create pipe");
  }
}

Pipe::Pipe(Pipe &&other) : _pipe_fds{-1, -1} {
  std::swap(_pipe_fds, other._pipe_fds);
}

Pipe::~Pipe() {
  for (auto &fd : _pipe_fds) {
    DTOR_TRY
    if (fd != -1) {
      close(fd);
      fd = -1;
    }
    DTOR_CATCH
  }
}

int Pipe::get_read_fd() {
  return _pipe_fds[0];
}

int Pipe::get_write_fd() {
  return _pipe_fds[1];
}
