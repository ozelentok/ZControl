#pragma once

#include "Worker.hpp"
#include "Server.hpp"
#include "doctest/doctest.h"
#include <chrono>
#include <thread>
#include <cstring>
#include <sys/stat.h>

using namespace std::chrono_literals;

static constexpr auto MaxRetries = 5;
static constexpr std::string Host = "127.0.0.1";
static constexpr auto Port = 5000;
static constexpr auto WaitTime = 20ms;

#define SETUP_COMMANDER_WORKER                                                                                         \
  Server server(Host, Port);                                                                                           \
  Worker worker(Host, Port);                                                                                           \
                                                                                                                       \
  uint32_t retry_counter = 0;                                                                                          \
  std::shared_ptr<Commander> cmdr = server.get_commander(server.get_clients()[0]);                                     \
  while (cmdr == nullptr) {                                                                                            \
    if (retry_counter >= MaxRetries) {                                                                                 \
      REQUIRE_MESSAGE(cmdr != nullptr, "No Commander connected to worker");                                            \
    }                                                                                                                  \
    ++retry_counter;                                                                                                   \
    std::this_thread::sleep_for(WaitTime);                                                                             \
    cmdr = server.get_commander(server.get_clients()[0]);                                                              \
  }

#define ERRNO_FMT(func_name, error_number)                                                                             \
  std::format(func_name "() errno {} {}", error_number, std::strerror(error_number))
