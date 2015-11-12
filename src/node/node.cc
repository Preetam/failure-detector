#include "node.hpp"
#include "../message/message.hpp"
#include "../message/identity_message.hpp"
#include "../message/ping_pong_message.hpp"

#include <chrono>

int
Node :: start(std::string address) {
	cpl::net::SockAddr addr;
	int status;
	status = addr.parse(address);
	if (status < 0) {
		return status;
	}
	status = sock.bind(addr);
	if (status < 0) {
		return status;
	}
	status = sock.listen();
	if (status < 0) {
		return status;
	}
	listen_address = address;
	return 0;
}

void
Node :: run() {
	std::thread cleanup([this]() {
		cleanup_nodes();
	});
	std::thread conn_accept_thread([this]() {
		handle_new_connections();
	});
	
	while (true) {
		auto start = std::chrono::steady_clock::now();
		std::lock_guard<cpl::Mutex> lk(*peers_lock);
		// This is the main node loop.
		process_message();

		// Cleanup check.
		for (int i = 0; i < peers->size(); i++) {
			auto peer = (*peers)[i];
			if (!peer->valid && !peer->active) {
				close_notify_sem->release();
				continue;
			}
		}

		// Pinging.
		for (int i = 0; i < peers->size(); i++) {
			auto peer = (*peers)[i];
			if (!peer->valid) {
				continue;
			}
			if (peer->active &&
				peer->ms_since_last_active() > 1000) {
				LOG(INFO) << "sending a ping to " << peer->address <<
					" because it was last active " << peer->ms_since_last_active() <<
					" ms ago";
				auto ping = std::make_unique<PingMessage>();
				peer->send(std::move(ping));
			} else {
				DLOG(INFO) << "NOT sending a ping to " << peer->address;
			}
		}

		auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - start).count();
		if (dur > 1000) {
			LOG(WARNING) << "Main node loop iteration took over 1000 ms!";
		}
	}
}

void
Node :: cleanup_nodes() {
	while (true) {
		// Wait until we're signaled that a connection
		// is closed.
		close_notify_sem->acquire();
		std::lock_guard<cpl::Mutex> lk(*peers_lock);
		LOG(INFO) << "cleaning up invalid peers";
		for (int i = 0; i < peers->size(); i++) {
			auto peer = (*peers)[i];
			if (!peer->valid) {
				peers->erase(peers->begin()+i);
				i--;
			}
		}
	}
}

void
Node :: handle_new_connections() {
	while (true) {
		auto conn_ptr = std::make_unique<cpl::net::TCP_Connection>();
		int status = 0;
		if ( (status = sock.accept(conn_ptr.get())) == 0) {
			conn_ptr->set_timeout(1,0);
			on_accept(std::move(conn_ptr));
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				continue;
			} else {
				break;
			}
		}
	}
}

void
Node :: on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr) {
	peers_lock->lock();
	auto peer = std::make_shared<Peer>(id_counter, std::move(conn_ptr), mq, close_notify_sem);
	peer->active = true;
	// Send our identity to the new peer.
	auto m = std::make_unique<IdentityMessage>(id, listen_address);
	peer->send(std::move(m));
	peers->push_back(std::move(peer));
	id_counter++;
	peers_lock->unlock();
}

bool
Node :: is_peered(uint64_t peer_id) {
	for (int i = 0; i < peers->size(); i++) {
		auto peer = (*peers)[i];
		if (peer->valid && peer->unique_id == peer_id) {
			return true;
		}
	}
	return false;
}

bool
Node :: is_active(uint64_t peer_id) {
	for (int i = 0; i < peers->size(); i++) {
		auto peer = (*peers)[i];
		if (peer->valid &&
			peer->active && peer->unique_id == peer_id) {
			return true;
		}
	}
	return false;
}
