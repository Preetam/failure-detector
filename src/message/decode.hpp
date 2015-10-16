#pragma once

#include <cassert>

#include "message.hpp"
#include "identity_message.hpp"
#include "ping_pong_message.hpp"
#include "suspect_message.hpp"

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
