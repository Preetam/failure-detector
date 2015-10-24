#include "peer.hpp"

#include "../message/decode.hpp"

void
Peer :: reconnect() {
	if (thread != nullptr && thread->joinable()) {
		thread->join();
		thread = nullptr;
	}
	LOG("attempting to reconnect to " << address);
	cpl::net::SockAddr addr;
	int status = addr.parse(address);
	if (status < 0) {
		return;
	}
	auto new_connection = std::make_unique<cpl::net::TCP_Connection>();
	status = new_connection->connect(addr);
	if (status < 0) {
		return;
	}
	conn = std::move(new_connection);
	last_update = std::chrono::steady_clock::now();
	thread = std::make_unique<std::thread>([this]() {
		read_messages();
	});
	active = true;
}

void
Peer :: read_messages() {
	while (run_listener) {
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
		m->source = local_id;
		if (m->type == MSG_IDENT) {
			valid = true;
		}
		mq->push(std::move(m));
		last_update = std::chrono::steady_clock::now();
	}

	if (!run_listener) {
		// We weren't signaled to stop listening.
		active = false;
		close_notify_sem->release();
	}
}

void
Peer :: send(std::unique_ptr<Message> m) {
	uint8_t buf[16000];
	int packed_size = m->pack(buf, 16000);
	int status = conn->send(buf, packed_size, 0);
	if (status > 0) {
		active = true;
		last_update = std::chrono::steady_clock::now();
	} else {
		active = false;
	}
}
