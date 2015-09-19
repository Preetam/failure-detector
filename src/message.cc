#include <cstring>

#include "message.hpp"

int
Message :: pack(uint8_t* dest, int dest_len) {
	if (dest_len < length+1) {
		return -1;
	}

	write8be(type, dest);
	dest++;
	write32be(length, dest);
	dest += 4;

	memcpy(dest, data, length);
	return 0;
}

int
Message :: unpack(uint8_t* src, int src_len) {
	if (src_len < 5) {
		return -1;
	}

	type = read8be(src);
	src++;
	length = read32be(src);
	src += 4;
	if (src_len - 5 < length) {
		return -1;
	}

	memcpy(data, src, length);
	return 0;
}
