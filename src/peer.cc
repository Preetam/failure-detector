#include "peer.hpp"

void
Peer :: read_packets() {
	while (true) {
		uint8_t buf[16000];
		Message m;
		int len = conn_ptr->recv(buf, 16000, 0);
		if (len <= 0) {
			return;
		}
		int status = m.unpack(buf, len);
		if (status != 0) {
			std::cerr << "message unpack status: " << status << std::endl;
			return;
		}
		std::cerr << "got message of type " << (int)m.type << std::endl;
		if (m.type == MSG_PING) {
			Message m(MSG_PONG, "PONG", 4);
			int len = m.pack(buf, 16000);
			conn_ptr->send(buf, len, 0);
		}
	}
}
