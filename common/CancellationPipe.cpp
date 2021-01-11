#include "CancellationPipe.hpp"
#include <stdint.h>
#include <system_error>
#include <errno.h>

int CancellationPipe::get_read_fd() {
	return _pipe.get_read_fd();
}

void CancellationPipe::notify() {
	uint8_t data[] = {1};
	ssize_t result = ::write(_pipe.get_write_fd(), &data, sizeof(data) / sizeof(*data));
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to write to cancellation pipe");
	}
}

void CancellationPipe::clear() {
	uint8_t data[64];
	ssize_t result = ::read(_pipe.get_read_fd(), data, sizeof(data) / sizeof(*data));
	if (result == -1) {
		throw std::system_error(errno, std::system_category(), "Failed to clear cancellation pipe");
	}
}
