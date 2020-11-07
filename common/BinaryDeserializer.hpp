#pragma once
#include <string>
#include <vector>
#include <stdint.h>

class BinaryDeserializer {
	private:
		const std::vector<uint8_t> &_serialized;
		uint32_t _index;

		void _validate_no_overflow(uint32_t size) const;

	public:
		BinaryDeserializer(const std::vector<uint8_t> &serialized);
		BinaryDeserializer(const BinaryDeserializer&) = delete;
		BinaryDeserializer(BinaryDeserializer&&) = delete;
		~BinaryDeserializer() = default;
		uint32_t bytes_available() const;
		uint8_t deserialize_uint8();
		uint32_t deserialize_uint32();
		int32_t deserialize_int32();
		int64_t deserialize_int64();
		std::string deserialize_str();
		std::vector<uint8_t> deserialize_vector();
		std::vector<uint8_t> deserialize_vector(uint32_t size);
};
