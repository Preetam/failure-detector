#pragma once

#include <string>
#include <cstdint>

#include "encoding.h"

enum MESSAGE_TYPE
{
	// Invalid
	MSG_INVALID,
	// Ping
	MSG_PING,
	// Ping response (pong)
	MSG_PONG,
	// Identification request
	MSG_IDENT_REQUEST,
	// Identification
	MSG_IDENT,
	// Suspected node
	MSG_SUSPECT,
	// Still alive message (in
	// response to MSG_SUSPECT)
	MSG_STILL_ALIVE
};

struct Message
{
	uint8_t     type;
	std::string data;
	int         source;

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
