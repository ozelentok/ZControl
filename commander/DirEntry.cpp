#include "DirEntry.hpp"

DirEntry::DirEntry(uint32_t inode, uint8_t type, std::string &&name) :
	_inode(inode), _type(type) {
	std::swap(_name, name);
}

uint32_t DirEntry::inode() const {
	return _inode;
}

uint8_t DirEntry::type() const {
	return _type;
}

const std::string& DirEntry::name() const {
	return _name;
}
