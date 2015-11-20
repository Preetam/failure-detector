#include "peer.hpp"

#include "../message/decode.hpp"

#include <future>

void
Peer :: reconnect() {
	cpl::RWLock lk(connection_lock, cpl::RWLock::Writer);
	last_reconnect = std::chrono::steady_clock::now();
	LOG(INFO) << "attempting to reconnect to " << address;
	cpl::net::SockAddr addr;
	int status = addr.parse(address);
	if (status < 0) {
		return;
	}
	auto new_connection = std::make_unique<cpl::net::TCP_Connection>();
	status = new_connection->connect(addr);
	if (status < 0) {
		LOG(INFO) << "couldn't reconnect to " << address;
		has_valid_connection = false;
		return;
	}
	LOG(INFO) << "reconnected to " << address;
	new_connection->set_timeout(0, 100);
	conn = std::move(new_connection);
	last_update = std::chrono::steady_clock::now();
	has_valid_connection = true;
}

void
Peer :: run() {
	// This function runs on a separate thread.
	// It will not exit until the Peer is destructed.
	while (run_thread) {
		bool sleep = false;
		uint8_t buf[16000];
		int len = 0;
		std::unique_ptr<Message> m;
		{
			cpl::RWLock lk(connection_lock, cpl::RWLock::Reader);
			if (!has_valid_connection) {
				sleep = true;
			} else {
				while (send_mq->size() > 0) {
					auto outbound_msg = send_mq->pop();
					int packed_size = outbound_msg->pack(buf, 16000);
					conn->send(buf, packed_size, 0);
				}
				len = conn->recv(buf, 16000, 0);
			}
		}
		if (sleep) {
			//if (ms_since_last_reconnect() > 1000 && valid) {
			//	LOG(INFO) << "attempting to reconnect to " << address;
			//	reconnect();
			//	continue;
			//}
			using namespace std::literals;
			std::this_thread::sleep_for(1000ms);
			continue;
		}

		if (len <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			cpl::RWLock lk(connection_lock, cpl::RWLock::Writer);
			has_valid_connection = false;
			conn = nullptr;
			active = false;
			continue;
		}

		int status = decode_message(m, buf, len);
		if (status < 0) {
			cpl::RWLock lk(connection_lock, cpl::RWLock::Writer);
			has_valid_connection = false;
			conn = nullptr;
			active = false;
			continue;
		}
		LOG(INFO) << "got a message from " << local_id;
		active = true;
		m->source = local_id;
		mq->push(std::move(m));
	}

	active = false;
	valid = false;
	close_notify_sem->release();
}

void
Peer :: send(std::unique_ptr<Message> m) {
	send_mq->push(std::move(m));
}
