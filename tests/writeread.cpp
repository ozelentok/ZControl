#include "utils.hpp"
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fcntl.h>
#include <print>

namespace fs = std::filesystem;

TEST_CASE("write() and read()") {
  SETUP_COMMANDER_WORKER;

  fs::path tmp_dir("/tmp/zcontrol-test");
  fs::create_directory(tmp_dir);

  int32_t fd = -1;
  int32_t worker_errno = 0;

  std::string text = "Contents of files";

  SECTION("Write and read") {
    auto output_path = tmp_dir / "zcontrol-readwrite-test";

    std::tie(fd, worker_errno) = cmdr->open(output_path, O_CREAT | O_WRONLY);
    REQUIRE_MESSAGE(fd != -1, ERRNO_FMT("open", worker_errno));

    int32_t bytes_written = 0;
    std::tie(bytes_written, worker_errno) =
        cmdr->write(fd, reinterpret_cast<const uint8_t *>(text.c_str()), text.size());
    REQUIRE_MESSAGE(bytes_written == text.size(), ERRNO_FMT("write", worker_errno));

    bool close_result = false;
    std::tie(close_result, worker_errno) = cmdr->close(fd);
    CHECK_MESSAGE(close_result, ERRNO_FMT("close", worker_errno));

    std::tie(fd, worker_errno) = cmdr->open(output_path, O_RDONLY);
    REQUIRE_MESSAGE(fd != -1, ERRNO_FMT("open", worker_errno));

    std::vector<uint8_t> data_read(text.size());
    auto bytes_read = cmdr->read(fd, data_read.data(), data_read.size()).first;
    REQUIRE_MESSAGE(bytes_read == text.size(), ERRNO_FMT("read", worker_errno));
    CHECK(memcmp(text.c_str(), data_read.data(), bytes_read) == 0);

    std::tie(close_result, worker_errno) = cmdr->close(fd);
    CHECK_MESSAGE(close_result, ERRNO_FMT("close", worker_errno));
  }

  SECTION("Invalid file path") {
    auto output_path = tmp_dir / "non-existing-dir" / "non-existing-file";
    std::tie(fd, worker_errno) = cmdr->open(output_path, O_CREAT | O_WRONLY);
    REQUIRE(fd == -1);
    REQUIRE_MESSAGE(worker_errno == ENOENT, ERRNO_FMT("open", worker_errno));

    int32_t bytes_written = 0;
    std::tie(bytes_written, worker_errno) =
        cmdr->write(fd, reinterpret_cast<const uint8_t *>(text.c_str()), text.size());
    REQUIRE(bytes_written == -1);
    REQUIRE_MESSAGE(worker_errno == EBADF, ERRNO_FMT("read", worker_errno));
  }
}
