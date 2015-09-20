#pragma once

#include <cstdint>

#include "encoding.h"

enum MESSAGE_TYPE
{
	MSG_INVALID,
	MSG_PING,
	MSG_PONG
};

struct Message
{
	uint8_t  type;
	uint32_t length;
	uint8_t* data;

	Message()
	: type(0), length(0), data(nullptr)
	{
	}

	Message(uint8_t type, const char* msg_data, uint32_t length)
	: type(type), length(length)
	{
		data = new uint8_t[length];
		memcpy((void*)data, (void*)msg_data, length);
	}

	int
	pack(uint8_t* dest, int dest_len);

	int
	unpack(uint8_t* src, int src_len);

	~Message()
	{
		if (data != nullptr) {
			delete[] data;
		}
	}
};
