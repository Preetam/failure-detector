#pragma once

#include <cstring>
#include <cassert>

#include "message.hpp"

class SuspectMessage : public Message
{
public:
	SuspectMessage()
	: Message(MSG_SUSPECT)
	{
	}

	SuspectMessage(uint64_t suspect)
	: Message(MSG_SUSPECT), suspect(suspect)
	{
	}

	inline int
	body_size()
	{
		return 8;
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) {
		if (dest_len < 8) {
			return -1;
		}
		write64be(suspect, dest);
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		if (src_len < 8) {
			return -1;
		}
		suspect = read64be(src);
		return 0;
	}

public:
	uint64_t suspect;
};

class StillAliveMessage : public Message
{
public:
	StillAliveMessage()
	: Message(MSG_STILL_ALIVE)
	{
	}

	inline int
	body_size()
	{
		return 0;
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) {
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		return 0;
	}
};
