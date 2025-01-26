#include "BinarySerializer.hpp"
#include <cstring>
#include <stdexcept>

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

void BinarySerializer::serialize_int64(int64_t value) {
  _serialized.resize(_index + sizeof(value));
  _serialized[_index] = ((value >> 56) & 0xFF);
  _serialized[_index + 1] = ((value >> 48) & 0xFF);
  _serialized[_index + 2] = ((value >> 40) & 0xFF);
  _serialized[_index + 3] = ((value >> 32) & 0xFF);
  _serialized[_index + 4] = ((value >> 24) & 0xFF);
  _serialized[_index + 5] = ((value >> 16) & 0xFF);
  _serialized[_index + 6] = ((value >> 8) & 0xFF);
  _serialized[_index + 7] = (value & 0xFF);
  _index += 8;
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

void BinarySerializer::serialize_byte_vector(const std::vector<uint8_t> &value) {
  const uint32_t length = value.size();
  serialize_uint32(length);
  _serialized.reserve(_index + length);
  _serialized.insert(_serialized.end(), value.begin(), value.end());
  _index += length;
}

void BinarySerializer::serialize_bytes(const uint8_t *bytes, uint32_t length) {
  serialize_uint32(length);
  _serialized.reserve(_index + length);
  _serialized.insert(_serialized.end(), bytes, bytes + length);
  _index += length;
}

std::vector<uint8_t> BinarySerializer::data() {
  std::vector<uint8_t> serialized;

  std::swap(serialized, _serialized);
  _index = 0;

  return serialized;
}
