#include <cstring>

#include "message.hpp"

int
Message :: pack(uint8_t* dest, int dest_len) {
	int length = body_size();
	if (dest_len < length+1) {
		return -1;
	}

	write8be(type, dest);
	dest++;
	dest_len--;
	write32be(length, dest);
	dest += 4;
	dest_len -= 4;

	int status = pack_body(dest, dest_len);
	if (status < 0) {
		return -2;
	}
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
	unpack_body(src, length);
	return 0;
}
