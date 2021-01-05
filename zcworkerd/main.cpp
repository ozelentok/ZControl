#include "Worker.hpp"
#include <thread>
#include <csignal>
#include <cstdio>

static std::function<void()> shutdown_function;

static void worker(const std::string &host, const uint16_t port) {
	Worker worker(host, port);
	std::thread worker_thread(&Worker::work, &worker);
	shutdown_function = [&]() {
		worker.stop();
  };
	std::signal(SIGINT, [](int) { shutdown_function(); });
	std::signal(SIGTERM, [](int) { shutdown_function(); });
	worker_thread.join();
}

int main(int argc, char const* argv[])
{
	try {
		if (argc < 3) {
			printf("usage: zcworkerd [HOST] [PORT]\n");
			return 1;
		}
		worker(argv[1], std::stoi(argv[2]));
		return 0;
	} catch (std::exception &e) {
		printf("Exception: %s\n", e.what());
	} catch (...) {
		printf("Unknown exception\n");
	}
}
