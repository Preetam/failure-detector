#pragma once

#include <chrono>
#include <thread>
#include <memory>
#include <cerrno>

#include <uv.h>
#include <glog/logging.h>
#include <cpl/net/sockaddr.hpp>

#include "peer/peer.hpp"
#include "peer_registry.hpp"

class Node
{
public:
	Node(uint64_t id)
	: m_id(id)
	, m_peer_registry(std::make_unique<PeerRegistry>())
	{
	}

	// start starts a node listening at address.
	// A negative value is returned for errors.
	int
	start(std::string address);

	// run starts the main processing routine.
	void
	run();

	// connect_to_peer connects to a peer with the given
	// address and creates a new Peer instance for the node.
	// If the node is unable to establish a connection, the
	// peer is registered as a failed node.
	void
	connect_to_peer(const cpl::net::SockAddr&);

private:
	uint64_t                              m_id;
	std::string                           m_listen_address;
	std::unique_ptr<uv_loop_t>            m_uv_loop;
	std::unique_ptr<uv_tcp_t>             m_tcp;
	std::unique_ptr<PeerRegistry>         m_peer_registry;
	int                                   m_index_counter;

	uint64_t                              m_trusted_peer;
	std::chrono::steady_clock::time_point m_last_leader_active;

	static void
	on_connect(uv_stream_t* server, int status) {
		auto self = (Node*)server->data;
		if (status < 0) {
			return;
		}
		auto client = std::make_unique<uv_tcp_t>();
		uv_tcp_init(server->loop, client.get());
		uv_accept(server, (uv_stream_t*)client.get());
		auto peer = std::make_shared<Peer>(std::move(client));
		peer->run();
		self->m_peer_registry->register_peer(++self->m_index_counter, peer);
	}
}; // Node
