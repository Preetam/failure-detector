#include "peer.hpp"

#include "../message/decode.hpp"

void
Peer :: reconnect() {
	cpl::RWLock lk(connection_lock, false);
	last_reconnect = std::chrono::steady_clock::now();
	LOG("attempting to reconnect to " << address);
	cpl::net::SockAddr addr;
	int status = addr.parse(address);
	if (status < 0) {
		return;
	}
	auto new_connection = std::make_unique<cpl::net::TCP_Connection>();
	status = new_connection->connect(addr);
	if (status < 0) {
		LOG("couldn't reconnect to " << address);
		has_valid_connection = false;
		return;
	}
	LOG("reconnected to " << address);
	new_connection->set_timeout(1,0);
	conn = std::move(new_connection);
	last_update = std::chrono::steady_clock::now();
	has_valid_connection = true;
}

void
Peer :: read_messages() {
	// This function runs on a separate thread.
	// It will not exit until the Peer is destructed.
	while (run_listener) {
		bool sleep = false;
		uint8_t buf[16000];
		int len = 0;
		std::unique_ptr<Message> m;
		{
			cpl::RWLock lk(connection_lock, true);
			if (!has_valid_connection) {
				sleep = true;
			} else {
				len = conn->recv(buf, 16000, 0);
			}
		}
		if (sleep) {
			using namespace std::literals;
			std::this_thread::sleep_for(1000ms);
			continue;
		}

		if (len <= 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;
			}
			connection_lock.lock();
			has_valid_connection = false;
			conn = nullptr;
			active = false;
			connection_lock.unlock();
			continue;
		}

		int status = decode_message(m, buf, len);
		if (status < 0) {
			connection_lock.lock();
			has_valid_connection = false;
			conn = nullptr;
			active = false;
			connection_lock.unlock();
			continue;
		}
		active = true;
		m->source = local_id;
		mq->push(std::move(m));
		last_update = std::chrono::steady_clock::now();
	}

	active = false;
	valid = false;
	close_notify_sem->release();
}

void
Peer :: send(std::unique_ptr<Message> m) {
	cpl::RWLock lk(connection_lock, true);
	if (!has_valid_connection) {
		return;
	}
	uint8_t buf[16000];
	int packed_size = m->pack(buf, 16000);
	conn->send(buf, packed_size, 0);
}
