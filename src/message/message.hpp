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

enum MESSAGE_FLAG
{
	FLAG_BCAST  = 1 << 0,
	FLAG_RBCAST = 1 << 1
};

class Message
{
public:
	Message()
	: type(MSG_INVALID)
	{
	}

	Message(uint8_t type)
	: type(type)
	{
	}

	Message(uint8_t type, uint8_t flags)
	: type(type), flags(flags)
	{
	}

	int
	pack(uint8_t* dest, int dest_len);

	int
	unpack(uint8_t* src, int src_len);

	inline bool
	broadcast()
	{
		return flags & (FLAG_BCAST|FLAG_RBCAST);
	}

	inline bool
	rbroadcast()
	{
		return flags & FLAG_RBCAST;
	}

	// Virtual methods to override
	virtual int
	body_size()
	{
		return 0;
	}

	virtual int
	pack_body(uint8_t* dest, int dest_len) = 0;

	virtual int
	unpack_body(uint8_t* src, int src_len) = 0;

public:
	int source;
	uint8_t type;
	uint8_t flags;
};
