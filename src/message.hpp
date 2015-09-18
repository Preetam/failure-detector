#pragma once

#include <cstdint>

struct Message
{
	uint8_t  type;
	uint32_t length;
	uint8_t* data;

	int
	pack(uint8_t* dest, int dest_len);
};
