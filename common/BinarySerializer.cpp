#include "BinarySerializer.hpp"
#include <stdexcept>
#include <cstring>
	

BinarySerializer::BinarySerializer() : _index(0) {}

void BinarySerializer::serialize_uint8(uint8_t value) {
	_serialized.resize(_index + sizeof(value));
	_serialized[_index] = value;
	_index += 1;
}

void BinarySerializer::serialize_uint32(uint32_t value) {
	_serialized.resize(_index + sizeof(value));
	_serialized[_index] = ((value >> 24) & 0xFF);
	_serialized[_index + 1] = ((value >> 16) & 0xFF);
	_serialized[_index + 2] = ((value >> 8) & 0xFF);
	_serialized[_index + 3] = (value & 0xFF);
	_index += 4;
}

void BinarySerializer::serialize_int32(int32_t value) {
	_serialized.resize(_index + sizeof(value));
	_serialized[_index] = ((value >> 24) & 0xFF);
	_serialized[_index + 1] = ((value >> 16) & 0xFF);
	_serialized[_index + 2] = ((value >> 8) & 0xFF);
	_serialized[_index + 3] = (value & 0xFF);
	_index += 4;
}

void BinarySerializer::serialize_str(const std::string &value) {
	const uint32_t length = value.length();
	serialize_uint32(length);
	_serialized.reserve(_index + length);
	_serialized.insert(_serialized.end(), value.begin(), value.end());
	_index += length;
}

void BinarySerializer::serialize_str(const char *value) {
	if (value == nullptr) {
		throw std::invalid_argument("NULL Parameter");
	}
	const uint32_t length = ::strlen(value);
	serialize_uint32(length);
	_serialized.reserve(_index + length);
	_serialized.insert(_serialized.end(), value, value + length);
	_index += length;
}

void BinarySerializer::serialize_vector(const std::vector<uint8_t> &value) {
	const uint32_t length = value.size();
	serialize_uint32(length);
	_serialized.reserve(_index + length);
	_serialized.insert(_serialized.end(), value.begin(), value.end());
	_index += length;
}

std::vector<uint8_t> BinarySerializer::data() const {
	return _serialized;
}
