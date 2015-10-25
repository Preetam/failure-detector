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
		peers_lock->lock();
		// This is the main node loop.
		process_message();

		// Reconnection check.
		for (int i = 0; i < peers->size(); i++) {
			auto peer = (*peers)[i];
			if (peer->valid && !peer->active &&
				peer->ms_since_last_active() > 5000) {
				// Attempt to reconnect.
				LOG("reconnecting to " << peer->address);
				peer->reconnect();
				// Send our identity to the new peer.
				auto m = std::make_unique<IdentityMessage>(id, listen_address);
				peer->send(std::move(m));
			}
		}

		// Pinging.
		for (int i = 0; i < peers->size(); i++) {
			auto peer = (*peers)[i];
			LOG(peer->address <<
				"[" << peer->local_id << "]" <<
				" was last active " << peer->ms_since_last_active() << " ms ago");
			if (peer->valid && peer->active &&
				peer->ms_since_last_active() > 1000) {
				LOG("sending a ping to " << peer->address);
				auto ping = std::make_unique<PingMessage>();
				peer->send(std::move(ping));
			}

			if (peer->ms_since_last_active() > 2000) {
				LOG(peer->address << " has not been active for over 2 sec.");
				peer->active = false;
			}
		}

		peers_lock->unlock();
	}
}

void
Node :: cleanup_nodes() {
	while (true) {
		// Wait until we're signaled that a connection
		// is closed.
		close_notify_sem->acquire();
		peers_lock->lock();
		LOG("Cleaning up invalid peers");
		for (int i = 0; i < peers->size(); i++) {
			auto peer = (*peers)[i];
			LOG("Peer " << peer->address <<
				" [" << peer->local_id << "]: " << peer->valid << "/" << peer->active);
			if (!peer->valid) {
				peers->erase(peers->begin()+i);
				i--;
				LOG("erasing invalid peer");
			}
		}
		peers_lock->unlock();
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
	peer->valid = true;
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
