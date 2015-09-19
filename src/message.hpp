#pragma once

#include <cstdint>

#include "encoding.h"

struct Message
{
	uint8_t  type;
	uint32_t length;
	uint8_t* data;

	int
	pack(uint8_t* dest, int dest_len);

	int
	unpack(uint8_t* src, int src_len);
};
