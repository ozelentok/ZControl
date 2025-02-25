#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "Worker.hpp"
#include "Server.hpp"
#include <thread>

int factorial(int number) {
  return number <= 1 ? number : factorial(number - 1) * number;
}

TEST_CASE("Testing stat") {
  auto host = "127.0.0.1";
  auto port = 5000;
  Server server(host, port);
  server.start();

  Worker worker(host, port);
  std::thread worker_thread(&Worker::work, &worker);
  auto cmdr = server.get_commander(server.get_clients()[0]);

  struct stat file_info = {0};
  int32_t ret = 0, worker_errno = 0;
  std::tie(ret, worker_errno) = cmdr->getattr("/tmp", file_info);
  CHECK(ret);
  CHECK(worker_errno == 0);
  CHECK((file_info.st_mode & S_IFDIR) == S_IFDIR);

  std::tie(ret, worker_errno) = cmdr->getattr("/DoesNotExist", file_info);
  CHECK(!ret);
  CHECK(worker_errno == ENOENT);
  CHECK((file_info.st_mode & S_IFDIR) == S_IFDIR);

  worker.stop();
  worker_thread.join();
}
