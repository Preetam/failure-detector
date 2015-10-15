#include "node.hpp"

#include "../message/ping_pong_message.hpp"

void
Node :: process_message() {
	std::unique_ptr<Message> m;
	if (mq->pop_with_timeout(m, 333)) {
		std::cerr << "new message" << std::endl;
		peers_lock->lock();
		switch (m->type) {
		case MSG_PING:
			handle_ping(m.get());
			break;
		case MSG_IDENT:
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
		if (peer->id == m->source) {
			auto response = std::make_unique<PongMessage>();
			peer->send(std::move(response));
		}
	}
}
