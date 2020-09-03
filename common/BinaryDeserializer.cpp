#include "BinaryDeserializer.hpp"

BinaryDeserializer::BinaryDeserializer(const std::vector<uint8_t> &serialized) : 
	_serialized(serialized), _index(0) {}

uint8_t BinaryDeserializer::deserialize_uint8() {
	return _serialized[_index];
}

uint32_t BinaryDeserializer::deserialize_uint32() {
	uint32_t value = 0;
	for (uint32_t i = 1; i <= sizeof(value); i++, _index++) {
		value += _serialized[_index] << (8 * (sizeof(value) - i));
	}
	return value;
}

std::string BinaryDeserializer::deserialize_str() {
	const uint32_t str_length = deserialize_uint32();
	std::string value(_serialized.begin() + _index, _serialized.begin() + _index + str_length);
	_index += str_length;
	return value;
}
