#include "BinarySerializer.hpp"

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

void BinarySerializer::serialize_str(const std::string &value) {
	const uint32_t str_length = value.length();
	serialize_uint32(str_length);
	_serialized.reserve(_index + str_length);
	_serialized.insert(_serialized.end(), value.begin(), value.end());
	_index += str_length;
}

std::vector<uint8_t> BinarySerializer::data() const {

	return _serialized;
}
