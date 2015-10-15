#pragma once

#include <cassert>

#include "message.hpp"

class PingMessage : public Message
{
public:
	PingMessage()
	: Message(MSG_PING)
	{
	}

	int
	body_size()
	{
		return 0;
	}

	int
	pack_body(uint8_t* dest, int dest_len) {
		return 0;
	}

	int
	unpack_body(uint8_t* src, int src_len)
	{
		return 0;
	}
};

class PongMessage : public Message
{
public:
	PongMessage()
	: Message(MSG_PONG)
	{
	}

	int
	body_size()
	{
		return 0;
	}

	int
	pack_body(uint8_t* dest, int dest_len) {
		return 0;
	}

	int
	unpack_body(uint8_t* src, int src_len)
	{
		return 0;
	}
};

