#pragma once

#include <cassert>

#include "message.hpp"

class IdentityMessage : public Message
{
public:
	IdentityMessage()
	: Message(MSG_IDENT)
	{
	}

	int
	body_size()
	{
		return 4 + address.size();
	}

	int
	pack_body(uint8_t* dest, int dest_len) {
		return -1;
	}

	int
	unpack_body(uint8_t* src, int src_len)
	{
		return -1;
	}

private:
	uint64_t id;
	std::string address;
};
