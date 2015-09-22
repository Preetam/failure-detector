#include <cstring>

#include "message.hpp"

int
Message :: pack(uint8_t* dest, int dest_len) {
	int length = data.length();
	if (dest_len < length+1) {
		return -1;
	}

	write8be(type, dest);
	dest++;
	write32be(length, dest);
	dest += 4;

	memcpy(dest, data.c_str(), (size_t)length);
	return 5+length;
}

int
Message :: unpack(uint8_t* src, int src_len) {
	if (src_len < 5) {
		return -1;
	}

	type = read8be(src);
	src++;
	uint32_t length = read32be(src);
	src += 4;
	if (src_len - 5 < length) {
		return -2;
	}
	data = std::string((const char*)src, (size_t)length);
	return 0;
}
