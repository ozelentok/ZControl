#include "AddressInfoErrorCategory.hpp"
#include <netdb.h>
#include <string.h>

AddressInfoErrorCategory::AddressInfoErrorCategory(int errno_value) : _errno(errno_value) {}
const char *AddressInfoErrorCategory::name() const noexcept {
  return "GAIError";
}

std::string AddressInfoErrorCategory::message(int gai_error) const {
  if (gai_error == EAI_SYSTEM) {
    return strerror(this->_errno);
  }
  return gai_strerror(errno);
}
