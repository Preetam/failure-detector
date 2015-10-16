#include "peer.hpp"

#include "../message/decode.hpp"

void
Peer :: read_messages() {
	while (true) {
		uint8_t buf[16000];
		std::unique_ptr<Message> m;
		int len = conn->recv(buf, 16000, 0);
		if (len == 0) {
			break;
		}
		if (len <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			break;
		}

		int status = decode_message(m, buf, len);
		if (status < 0) {
			continue;
		}
		m->source = id;
		mq->push(std::move(m));
	}
	active = false;
	close_notify_sem->release();
}

void
Peer :: send(std::unique_ptr<Message> m) {
	uint8_t buf[16000];
	int packed_size = m->pack(buf, 16000);
	conn->send(buf, packed_size, 0);
}
