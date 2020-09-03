#pragma once
#include <string>
#include <vector>
#include <stdint.h>

class BinaryDeserializer {
	private:
		const std::vector<uint8_t> &_serialized;
		uint32_t _index;

	public:
		BinaryDeserializer(const std::vector<uint8_t> &serialized);
		BinaryDeserializer(const BinaryDeserializer&) = delete;
		BinaryDeserializer(BinaryDeserializer&&) = delete;
		~BinaryDeserializer() = default;
		uint8_t deserialize_uint8();
		uint32_t deserialize_uint32();
		std::string deserialize_str();
};
