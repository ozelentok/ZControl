#include "AddressInfo.hpp"
#include "AddressInfoErrorCategory.hpp"
#include "Logger.hpp"
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

AddressInfo::~AddressInfo() {
  DTOR_TRY
  if (_info != nullptr) {
    freeaddrinfo(_info);
    _info = nullptr;
  }
  DTOR_CATCH
}

const addrinfo *AddressInfo::get() const {
  return _info;
}

bool AddressInfo::is_empty() const {
  return _info == nullptr;
}
