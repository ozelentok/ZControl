#pragma once

#include "Worker.hpp"
#include "Server.hpp"
#include "doctest/doctest.h"
#include <stdexcept>
#include <cstring>
#include <sys/stat.h>

#define SETUP_COMMANDER_WORKER                                                                                         \
  auto host = "127.0.0.1";                                                                                             \
  auto port = 5000;                                                                                                    \
  Server server(host, port);                                                                                           \
  Worker worker(host, port);                                                                                           \
  auto cmdr = server.get_commander(server.get_clients()[0])

#define ERRNO_FMT(func_name, error_number) std::format(func_name "() errno {} {}", error_number, std::strerror(error_number))
