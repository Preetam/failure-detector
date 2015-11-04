#pragma once

#include <string>
#include <cstdint>
#include <random>
#include <chrono>

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

inline
const char* MSG_STR(uint8_t type) {
	switch (type) {
	case MSG_PING: return "MSG_PING";
	case MSG_PONG: return "MSG_PONG";
	case MSG_IDENT_REQUEST: return "MSG_IDENT_REQUEST";
	case MSG_IDENT: return "MSG_IDENT";
	case MSG_SUSPECT: return "MSG_SUSPECT";
	case MSG_STILL_ALIVE: return "MSG_STILL_ALIVE";
	}

	return "MSG_INVALID";
}

enum MESSAGE_FLAG
{
	FLAG_BCAST  = 1 << 0,
	FLAG_RBCAST = 1 << 1
};

// Initialize RNG
static std::mt19937_64 rng(
	std::chrono::system_clock::now().time_since_epoch().count());

class Message
{
public:
	Message()
	: type(MSG_INVALID)
	{
	}

	Message(uint8_t type)
	: type(type), flags(0), id(rng())
	{
	}

	Message(uint8_t type, uint8_t flags)
	: type(type), flags(flags), id(rng())
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
	uint64_t id;
};
