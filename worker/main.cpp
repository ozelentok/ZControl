#include "Worker.hpp"
#include <cstdio>

void worker(const std::string &host, const uint16_t port) {
	Worker worker(host, port);
	worker.work();
}

int main(int argc, char const* argv[])
{
	try {
		if (argc < 3) {
			printf("usage: worker [HOST] [PORT]\n");
			return 1;
		}
		worker(argv[1], std::stoi(argv[2]));
		return 0;
	} catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}
}
