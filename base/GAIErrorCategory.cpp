#include "GAIErrorCategory.hpp"
#include <netdb.h>
#include <string.h>

GAIErrorCategory::GAIErrorCategory(int errno_value) : _errno(errno_value) {}
const char *GAIErrorCategory::name() const noexcept {
	return "GAIError";
}

std::string GAIErrorCategory::message(int gai_error) const {
	if (gai_error == EAI_SYSTEM) {
		return strerror(this->_errno);
	}
	return gai_strerror(errno);
}
