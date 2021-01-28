#pragma once
#include <string>
#include <netdb.h>

class AddressInfo {
private:
  addrinfo *_info;

public:
  AddressInfo(const std::string &host, uint16_t port);
  AddressInfo(const AddressInfo &other) = delete;
  AddressInfo(AddressInfo &&other);
  AddressInfo &operator=(const AddressInfo &other) = delete;
  AddressInfo &operator=(AddressInfo &&other) = delete;
  ~AddressInfo();
  const addrinfo *get() const;
  bool is_empty() const;
};
