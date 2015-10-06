#include "peer.hpp"

void
Peer :: read_messages() {
	while (true) {
		uint8_t buf[16000];
		Message m;
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
		int status = m.unpack(buf, len);
		if (status != 0) {
			break;
		}
		m.source = id;
		mq->push(m);
	}
	active = false;
	close_notify_sem->release();
}

void
Peer :: send(Message m) {
	uint8_t buf[16000];
	int packed_size = m.pack(buf, 16000);
	conn->send(buf, packed_size, 0);
}
