#pragma once
#include <string>
#include <netdb.h>
#include "GAIErrorCategory.hpp"

class AddressInfo {
	private:
		addrinfo *_info;

	public:
		AddressInfo(const std::string &host, uint16_t port);
		AddressInfo(const AddressInfo&) = delete;
		~AddressInfo();
		const addrinfo* get() const;
		bool is_empty() const;
};
