#include "node.hpp"

void
Node :: process_message() {
	Message m;
	if (mq->pop_with_timeout(&m, 33)) {
		std::cout << "new message" << std::endl;
		if (m.type == MSG_PING) {
			// Send a PONG.
			peers_lock->lock();
			for (int i = 0; i < peers->size(); i++) {
				auto peer = (*peers)[i];
				if (peer->id == m.source) {
					m.type = MSG_PONG;
					peer->send(m);
				}
			}
			peers_lock->unlock();
		}
	}
}
