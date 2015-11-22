#include <memory>
#include <cstring>

#include "message.hpp"
#include "identity_message.hpp"
#include "ping_pong_message.hpp"
#include "suspect_message.hpp"

/**
 * A message header has 4 fields:
 * - type (1 byte)
 * - flags (1 byte)
 * - id (8 bytes)
 * - length (4 bytes)
 * Total: 14 bytes
 */
#define MSG_HEADER_SIZE 14

int
Message :: pack(uint8_t* dest, int dest_len) {
	int length = body_size();
	if (dest_len < length+MSG_HEADER_SIZE) {
		return -1;
	}

	write8be(type, dest);
	dest++;
	dest_len--;
	write8be(flags, dest);
	dest++;
	dest_len--;
	write64be(id, dest);
	dest += 8;
	dest_len -= 8;
	write32be(length, dest);
	dest += 4;
	dest_len -= 4;

	int status = pack_body(dest, dest_len);
	if (status < 0) {
		return -2;
	}
	return MSG_HEADER_SIZE+length;
}

int
Message :: unpack(uint8_t* src, int src_len) {
	if (src_len < MSG_HEADER_SIZE) {
		return -1;
	}

	type = read8be(src);
	src++;
	flags = read8be(src);
	src++;
	id = read64be(src);
	src += 8;
	uint32_t length = read32be(src);
	src += 4;
	if (src_len - MSG_HEADER_SIZE < length) {
		return -2;
	}
	unpack_body(src, length);
	return 0;
}

int
decode_message(std::unique_ptr<Message>& m, uint8_t* src, int src_len) {
	assert(src_len > 0);
	// Peek at the message type
	switch (src[0]) {
	case MSG_PING:
		m = std::make_unique<PingMessage>();
		break;
	case MSG_PONG:
		m = std::make_unique<PongMessage>();
		break;
	case MSG_IDENT_REQUEST:
		m = std::make_unique<IdentityRequest>();
		break;
	case MSG_IDENT:
		m = std::make_unique<IdentityMessage>();
		break;
	case MSG_SUSPECT:
		m = std::make_unique<SuspectMessage>();
		break;
	case MSG_STILL_ALIVE:
		m = std::make_unique<StillAliveMessage>();
		break;
	default:
		return -1;
	}

	return m->unpack(src, src_len);
}


#undef MSG_HEADER_SIZE
