#include "Worker.hpp"
#include <cstdio>

void worker() {
	Worker worker("127.0.0.1", 4444);
	worker.work();
}

int main(int argc, char const* argv[])
{
	try {
		worker();
		return 0;
	} catch (std::exception& e) {
		printf("Exception: %s\n", e.what());
	}
}
