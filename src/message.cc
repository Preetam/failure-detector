#include <cstring> // memcpy

#include "encoding.h"
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
