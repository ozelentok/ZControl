#include "BinaryDeserializer.hpp"
#include <stdexcept>

BinaryDeserializer::BinaryDeserializer(const std::vector<uint8_t> &serialized) :_serialized(serialized), _index(0) {
}

void BinaryDeserializer::_validate_no_overflow(uint32_t size) const {
	const uint32_t available_bytes_count = _serialized.size() - _index;
	if (size > available_bytes_count) {
		throw std::runtime_error("Expected size of serialized object is larger than available bytes: " +
															std::to_string(size) + " > " + std::to_string(available_bytes_count));
	}
}

uint8_t BinaryDeserializer::deserialize_uint8() {
	_validate_no_overflow(sizeof(uint8_t));
	return _serialized[_index++];
}

uint32_t BinaryDeserializer::deserialize_uint32() {
	uint32_t value = 0;
	_validate_no_overflow(sizeof(value));
	for (uint32_t i = 1; i <= sizeof(value); i++, _index++) {
		value += _serialized[_index] << (8 * (sizeof(value) - i));
	}
	return value;
}

int32_t BinaryDeserializer::deserialize_int32() {
	int32_t value = 0;
	_validate_no_overflow(sizeof(value));
	for (uint32_t i = 1; i <= sizeof(value); i++, _index++) {
		value += _serialized[_index] << (8 * (sizeof(value) - i));
	}
	return value;
}

std::string BinaryDeserializer::deserialize_str() {
	const uint32_t length = deserialize_uint32();
	_validate_no_overflow(length);
	std::string value(_serialized.begin() + _index, _serialized.begin() + _index + length);
	_index += length;
	return value;
}

std::vector<uint8_t> BinaryDeserializer::deserialize_vector() {
	const uint32_t length = deserialize_uint32();
	return deserialize_vector(length);
}

std::vector<uint8_t> BinaryDeserializer::deserialize_vector(uint32_t length) {
	_validate_no_overflow(length);
	std::vector<uint8_t> value(_serialized.begin() + _index, _serialized.begin() + _index + length);
	_index += length;
	return value;
}