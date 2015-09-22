#pragma once

#include <string>
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
	uint8_t     type;
	std::string data;

	Message()
	: type(0), data("")
	{
	}

	Message(uint8_t type, std::string data)
	: type(type), data(data)
	{
	}

	Message(uint8_t type, const char* data)
	: type(type), data(data)
	{
	}

	int
	pack(uint8_t* dest, int dest_len);

	int
	unpack(uint8_t* src, int src_len);
};
