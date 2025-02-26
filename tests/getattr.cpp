#include "utils.hpp"

TEST_CASE("getattr") {
  SETUP_COMMANDER_WORKER;

  struct stat file_info = {0};
  bool result = 0;
  int32_t worker_errno = 0;

  SUBCASE("Valid file path") {
    std::tie(result, worker_errno) = cmdr->getattr("/tmp", file_info);
    CHECK(result);
    CHECK(worker_errno == 0);
    CHECK(S_ISDIR(file_info.st_mode));

    std::tie(result, worker_errno) = cmdr->getattr("/usr/bin/bash", file_info);
    CHECK(result);
    CHECK(worker_errno == 0);
    CHECK(S_ISREG(file_info.st_mode));
  }

  SUBCASE("Invalid file path") {
    std::tie(result, worker_errno) = cmdr->getattr("/DoesNotExist", file_info);
    CHECK(!result);
    CHECK(worker_errno == ENOENT);

    std::tie(result, worker_errno) = cmdr->getattr("/usr", file_info);
    CHECK(result);
    CHECK(worker_errno == 0);
    CHECK(S_ISDIR(file_info.st_mode));
  }
}
