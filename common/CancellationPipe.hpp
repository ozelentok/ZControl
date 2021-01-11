#include "Pipe.hpp"
#include <stdexcept>

class CancellationException: public std::runtime_error {
	public:
		CancellationException(const std::string &what): std::runtime_error(what) {}
};

class CancellationPipe {
	private:
		Pipe _pipe;

	public:
		int get_read_fd();
		void notify();
		void clear();
};
