#pragma once

#include <cstring>
#include <cassert>

#include "message.hpp"

class IdentityRequest : public Message
{
public:
	IdentityRequest()
	: Message(MSG_IDENT_REQUEST)
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

class IdentityMessage : public Message
{
public:
	IdentityMessage()
	: Message(MSG_IDENT), id(0), address("")
	{
	}

	IdentityMessage(uint64_t id, std::string address)
	: Message(MSG_IDENT), id(id), address(address)
	{
	}

	inline int
	body_size()
	{
		return 8 + 2 + address.size();
	}

	inline int
	pack_body(uint8_t* dest, int dest_len) {
		if (dest_len < 8 + 2 + address.size()) {
			return -1;
		}
		write64be(id, dest);
		dest += 8;
		write16be(address.size(), dest);
		dest += 2;
		memcpy(dest, address.c_str(), address.size());
		return 0;
	}

	inline int
	unpack_body(uint8_t* src, int src_len)
	{
		if (src_len < 8 + 2) {
			return -1;
		}
		id = read64be(src);
		src += 8;
		uint16_t address_size = read16be(src);
		src += 2;
		if (src_len < 8 + 2 + address_size) {
			return -2;
		}
		address = std::string((const char*)src, address_size);
		return 0;
	}

public:
	uint64_t id;
	std::string address;
};
