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
		BinarySerializer(const BinarySerializer&) = delete;
		BinarySerializer(BinarySerializer&&) = delete;
		~BinarySerializer() = default;
		void serialize_uint8(uint8_t value);
		void serialize_uint32(uint32_t value);
		void serialize_int32(int32_t value);
		void serialize_int64(int64_t value);
		void serialize_str(const std::string &value);
		void serialize_str(const char *value);
		void serialize_vector(const std::vector<uint8_t> &value);

		std::vector<uint8_t> data() const;
};
