#pragma once
#include <string>
#include <stdint.h>

class DirEntry {
	private:
		uint32_t _inode;
		uint8_t _type;
		std::string _name;

	public:
		DirEntry(uint32_t inode, uint8_t type, std::string &&name);
		uint32_t inode() const;
		uint8_t type() const;
		const std::string &name() const;
};
