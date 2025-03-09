#include "utils.hpp"
#include <stdexcept>
#include <sys/stat.h>

TEST_CASE("Connection") {
  SETUP_COMMANDER_WORKER;

  struct stat file_info = {0};

  SECTION("Connection") {
    REQUIRE_NOTHROW(cmdr->getattr("/", file_info));
    REQUIRE_NOTHROW(cmdr->disconnect());
    REQUIRE_THROWS_AS(cmdr->getattr("/", file_info), std::runtime_error);
    REQUIRE_THROWS_AS(cmdr->getattr("/tmp", file_info), std::runtime_error);
  }

  worker.close();
  worker.wait();
  worker.wait();
  worker.close();

  SECTION("Disconnection") {
    REQUIRE_THROWS_AS(cmdr->getattr("/tmp", file_info), std::runtime_error);
    REQUIRE_THROWS_AS(cmdr->getattr("/DoesNotExist", file_info), std::runtime_error);
  }

}
