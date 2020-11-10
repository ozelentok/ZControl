#pragma once
#include <string>
#include <netdb.h>

class AddressInfo {
	private:
		addrinfo *_info;

	public:
		AddressInfo(const std::string &host, uint16_t port);
		AddressInfo(const AddressInfo&) = delete;
		//TODO: delete assigment operators
		~AddressInfo();
		const addrinfo* get() const;
		bool is_empty() const;
};
