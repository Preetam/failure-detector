#include "../message/identity_message.hpp"

#include "node.hpp"

void
Node :: connect_to_peer(cpl::net::SockAddr address) {
	peers_lock->lock();
	auto peer_conn = std::make_unique<cpl::net::TCP_Connection>();
	int status = peer_conn->connect(address);
	std::shared_ptr<Peer> peer;
	if (status < 0) {
		LOG("unable to connect to " << address);
		LOG("adding " << address << " as a failed peer.");
		peer = std::make_shared<Peer>(id_counter++, address.str(), mq, close_notify_sem);
	} else {
		LOG("successfully connected to " << address);
		peer = std::make_shared<Peer>(id_counter++, std::move(peer_conn), mq, close_notify_sem);
		// Send our identity to the new peer.
		auto m = std::make_unique<IdentityMessage>(id, listen_address);
		peer->send(std::move(m));
	}
	peers->push_back(std::move(peer));
	peers_lock->unlock();
}
