#pragma once

#include <chrono>
#include <thread>
#include <memory>
#include <cerrno>

#include <glog/logging.h>
#include <cpl/net/sockaddr.hpp>
#include <cpl/net/tcp_socket.hpp>
#include <cpl/semaphore.hpp>

#include "message/identity_message.hpp"
#include "message/message.hpp"
#include "message/identity_message.hpp"
#include "message/ping_pong_message.hpp"
#include "message_queue/message_queue.hpp"
#include "peer/peer.hpp"
#include "peer_registry.hpp"

class Node
{
public:
	Node(uint64_t id)
	: m_id(id),
	  m_mq(std::make_shared<Message_Queue>()),
	  m_registry(std::make_shared<PeerRegistry>()),
	  m_close_notify_sem(std::make_shared<cpl::Semaphore>(0))
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
	uint64_t                        m_id;
	std::string                     m_listen_address;
	cpl::net::TCP_Socket            m_sock;
	std::shared_ptr<Message_Queue>  m_mq;
	std::shared_ptr<PeerRegistry>   m_registry;
	// close_notify_sem notifies the cleanup thread
	// whenever a Peer connection closes.
	std::shared_ptr<cpl::Semaphore> m_close_notify_sem;
	int                             m_index_counter;

	// process_message processes a single message in the inbound
	// message queue. This function blocks up to 33 milliseconds.
	void
	process_message();

	// cleanup_peers runs in a separate thread to clean up
	// disconnected Peer instances.
	void
	cleanup_peers();

	// handle_new_connections runs in a separate thread to
	// accept new connections.
	void
	handle_new_connections();

	// on_accept is the function called by handle_new_connections
	// whenever a new connection is accepted.
	void
	on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr);

	// Message handlers
	void
	handle_ping(const Message&);
	void
	handle_pong(const Message&);
	void
	handle_ident(const Message&);
	void
	handle_ident_request(const Message&);
}; // Node
