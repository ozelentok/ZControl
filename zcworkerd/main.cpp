#include "Worker.hpp"
#include <csignal>
#include <print>

static std::function<void()> shutdown_function;

static void worker(const std::string &host, const uint16_t port) {
  Worker worker(host, port);
  shutdown_function = [&]() { worker.close(); };
  std::signal(SIGINT, [](int) { shutdown_function(); });
  std::signal(SIGTERM, [](int) { shutdown_function(); });
  worker.wait();
}

int main(int argc, char const *argv[]) {
  try {
    if (argc < 3) {
      std::println("usage: zcworkerd [HOST] [PORT]");
      return 1;
    }
    worker(argv[1], std::stoi(argv[2]));
    return 0;
  } catch (std::exception &e) {
    std::println("Exception: {}", e.what());
  } catch (...) {
    std::println("Unknown Exception");
  }
}
