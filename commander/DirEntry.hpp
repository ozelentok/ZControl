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
		DirEntry(const DirEntry &other) = default;
		DirEntry(DirEntry &&other) = default;
		~DirEntry() = default;
		uint32_t inode() const;
		uint8_t type() const;
		const std::string &name() const;
};
