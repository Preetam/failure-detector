#include "../message/identity_message.hpp"

#include "node.hpp"

void
Node :: connect_to_peer(cpl::net::SockAddr address) {
	peers_lock->lock();
	auto peer_conn = std::make_unique<cpl::net::TCP_Connection>();
	std::shared_ptr<Peer> peer;
	// Attempt to connect.
	int status = peer_conn->connect(address);
	if (status < 0) {
		LOG("unable to connect to " << address);
		LOG("adding " << address << " as a failed peer.");
		peer = std::make_shared<Peer>(id_counter++, address.str(), mq, close_notify_sem);
	} else {
		LOG("successfully connected to " << address);
		peer_conn->set_timeout(1,0);
		peer = std::make_shared<Peer>(id_counter++, std::move(peer_conn), address.str(), mq, close_notify_sem);
		// Send our identity to the new peer.
		auto m = std::make_unique<IdentityMessage>(id, listen_address);
		peer->send(std::move(m));
	}
	peers->push_back(std::move(peer));
	peers_lock->unlock();
}
