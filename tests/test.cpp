#include <cstdint>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "Worker.hpp"
#include "Server.hpp"

TEST_CASE("stat()") {
  auto host = "127.0.0.1";
  auto port = 5000;
  Server server(host, port);

  Worker worker(host, port);
  auto cmdr = server.get_commander(server.get_clients()[0]);

  struct stat file_info = {0};
  bool ret = 0;
  int32_t worker_errno = 0;

  SUBCASE("dir exists") {
    std::tie(ret, worker_errno) = cmdr->getattr("/tmp", file_info);
    CHECK(ret);
    CHECK(worker_errno == 0);
    CHECK((file_info.st_mode & S_IFDIR) == S_IFDIR);
  }

  SUBCASE("does not exist and exists") {
    std::tie(ret, worker_errno) = cmdr->getattr("/DoesNotExist", file_info);
    CHECK(!ret);
    CHECK(worker_errno == ENOENT);

    std::tie(ret, worker_errno) = cmdr->getattr("/usr", file_info);
    CHECK(ret);
    CHECK(worker_errno == 0);
    CHECK((file_info.st_mode & S_IFDIR) == S_IFDIR);
  }

  worker.close();
  worker.wait();

  SUBCASE("disconnected error") {
    REQUIRE_THROWS_WITH(cmdr->getattr("/tmp", file_info), "Disconnected");
    REQUIRE_THROWS_WITH(cmdr->getattr("/DoesNotExist", file_info), "Disconnected");
  }
}
