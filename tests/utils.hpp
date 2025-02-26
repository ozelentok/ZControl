#pragma once

#include "Worker.hpp"
#include "Server.hpp"
#include "doctest/doctest.h"
#include <stdexcept>
#include <sys/stat.h>

#define SETUP_COMMANDER_WORKER                                                                                         \
  auto host = "127.0.0.1";                                                                                             \
  auto port = 5000;                                                                                                    \
  Server server(host, port);                                                                                           \
  Worker worker(host, port);                                                                                           \
  auto cmdr = server.get_commander(server.get_clients()[0])
