#include "AddressInfo.hpp"
#include "AddressInfoErrorCategory.hpp"
#include "SysLog.hpp"
#include <errno.h>

AddressInfo::AddressInfo(const std::string &host, uint16_t port) {
  int result = getaddrinfo(host.c_str(), std::to_string(port).c_str(), nullptr, &(this->_info));
  if (result != 0) {
    throw std::system_error(result, AddressInfoErrorCategory(errno), "Failed to get address info");
  }
}

AddressInfo::AddressInfo(AddressInfo &&other) : _info(nullptr) {
  std::swap(_info, other._info);
}

const addrinfo *AddressInfo::get() const {
  return _info;
}

bool AddressInfo::is_empty() const {
  return _info == nullptr;
}

AddressInfo::~AddressInfo() {
  try {
    if (_info != nullptr) {
      freeaddrinfo(_info);
      _info = nullptr;
    }
  }
  CATCH_ALL_ERROR_HANDLER
}
