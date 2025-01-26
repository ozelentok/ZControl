#pragma once
#include <string>
#include <vector>
#include <stdint.h>

class BinarySerializer {
private:
  std::vector<uint8_t> _serialized;
  uint32_t _index;

public:
  BinarySerializer();
  BinarySerializer(const BinarySerializer &other) = delete;
  BinarySerializer(BinarySerializer &&other) = delete;
  BinarySerializer &operator=(const BinarySerializer &other) = delete;
  BinarySerializer &operator=(BinarySerializer &&other) = delete;
  ~BinarySerializer() = default;
  void serialize_uint8(uint8_t value);
  void serialize_uint32(uint32_t value);
  void serialize_int32(int32_t value);
  void serialize_int64(int64_t value);
  void serialize_str(const std::string &value);
  void serialize_str(const char *value);
  void serialize_byte_vector(const std::vector<uint8_t> &value);
  void serialize_bytes(const uint8_t *bytes, uint32_t length);

  std::vector<uint8_t> data();
};
