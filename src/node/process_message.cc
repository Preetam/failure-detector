#include "node.hpp"

#include "../message/message.hpp"
#include "../message/ping_pong_message.hpp"
#include "../message/identity_message.hpp"

void
Node :: process_message() {
	std::unique_ptr<Message> m;
	if (mq->pop_with_timeout(m, 500)) {
		LOG("new message (type " << MSG_STR(m->type) << ")");
		switch (m->type) {
		case MSG_PING:
			handle_ping(m.get());
			break;
		case MSG_IDENT:
			handle_ident(m.get());
			break;
		case MSG_IDENT_REQUEST:
			handle_ident_request(m.get());
			break;
		default:
			break;
		}
	}
}

void
Node :: handle_ping(const Message* m) {
	// Send a PONG.
	std::shared_ptr<Peer> peer;
	for (int i = 0; i < peers->size(); i++) {
		peer = (*peers)[i];
		if (peer->local_id == m->source) {
			auto response = std::make_unique<PongMessage>();
			peer->send(std::move(response));
		}
	}
	// Check if the sender is known as a valid peer.
	if (!peer->valid) {
		LOG("Got a PING from an invalid Peer! Requesting identity.");
		auto req = std::make_unique<IdentityRequest>();
		peer->send(std::move(req));
	}
}

void
Node :: handle_ident(const Message* m) {
	auto ident_msg = static_cast<const IdentityMessage*>(m);
	LOG("got an identity message from " << ident_msg->address << " source " << ident_msg->source);
	uint64_t peer_id = ident_msg->id;

	// Check if this is a preexisting peer
	if (is_peered(peer_id)) {
		LOG("Already peered with " << peer_id);
		// Is it inactive? If it is, then we probably have a new
		// connection and need to update the existing peer.
		if (!is_active(peer_id)) {
			LOG(peer_id << " is not active. Will update connection.");
			// Update the connection.
			std::shared_ptr<Peer> existing_peer;
			std::shared_ptr<Peer> new_peer;
			for (int i = 0; i < peers->size(); i++) {
				auto peer = (*peers)[i];
				if (peer->local_id == ident_msg->source) {
					new_peer = peer;
				}
				if (peer->unique_id == peer_id) {
					existing_peer = peer;
				}
			}
			new_peer->valid = false;
			new_peer->active = false;
			new_peer->stop();
			auto conn = std::move(new_peer->get_conn());
			existing_peer->update_conn(std::move(conn));
		} else {
			// Peer is active but we still got a new connection.
			// Discard the new one.
			LOG("Discarding new connection as a duplicate.");
			std::shared_ptr<Peer> new_peer;
			for (int i = 0; i < peers->size(); i++) {
				auto peer = (*peers)[i];
				if (peer->local_id == ident_msg->source) {
					new_peer = peer;
					break;
				}
			}
			new_peer->stop();
			new_peer->valid = false;
			close_notify_sem->release();
		}
	} else {
		LOG("Not peered with " << peer_id);
		// This peer has not been registered.
		std::shared_ptr<Peer> new_peer;
		LOG("peers->size() == " << peers->size());
		for (int i = 0; i < peers->size(); i++) {
			auto peer = (*peers)[i];
			LOG(peer->local_id << " " << ident_msg->source);
			if (peer->local_id == ident_msg->source) {
				peer->unique_id = peer_id;
				peer->address = ident_msg->address;
				LOG("Registered " << peer_id << " with source " << ident_msg->source);
				break;
			}
		}
	}
SKIP:
	for (int i = 0; i < peers->size(); i++) {
		auto peer = (*peers)[i];
		if (peer->local_id == m->source) {
			peer->unique_id = ident_msg->id;
			peer->address = ident_msg->address;
			peer->valid = true;
		}
	}
}

void
Node :: handle_ident_request(const Message* m) {
	auto ident_req = static_cast<const IdentityRequest*>(m);
	for (int i = 0; i < peers->size(); i++) {
		auto peer = (*peers)[i];
		if (peer->local_id == m->source) {
			// Just send our identity.
			auto m = std::make_unique<IdentityMessage>(id, listen_address);
			peer->send(std::move(m));
		}
	}
}
