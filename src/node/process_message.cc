#include "node.hpp"

#include "../message/message.hpp"
#include "../message/ping_pong_message.hpp"
#include "../message/identity_message.hpp"

void
Node :: process_message() {
	std::unique_ptr<Message> m;
	if (mq->pop_with_timeout(m, 500)) {
		LOG("new message (type " << MSG_STR(m->type) << ")");
		peers_lock->lock();
		switch (m->type) {
		case MSG_PING:
			handle_ping(m.get());
			break;
		case MSG_IDENT:
			handle_ident(m.get());
			break;
		default:
			break;
		}
		peers_lock->unlock();
	}
}

void
Node :: handle_ping(const Message* m) {
	// Send a PONG.
	for (int i = 0; i < peers->size(); i++) {
		auto peer = (*peers)[i];
		if (peer->index == m->source) {
			auto response = std::make_unique<PongMessage>();
			peer->send(std::move(response));
		}
	}
}

void
Node :: handle_ident(const Message* m) {
	auto ident_msg = static_cast<const IdentityMessage*>(m);
	LOG("got an identity message from " << ident_msg->address << " source " << ident_msg->source);
	for (int i = 0; i < peers->size(); i++) {
		auto peer = (*peers)[i];
		if (peer->index == m->source) {
			peer->unique_id = ident_msg->id;
			peer->address = ident_msg->address;
		}
	}
}
