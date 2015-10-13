#pragma once

#include <chrono>
#include <thread>
#include <memory>
#include <cerrno>

#include <cpl/net/sockaddr.hpp>
#include <cpl/net/tcp_socket.hpp>
#include <cpl/semaphore.hpp>
#include <cpl/mutex.hpp>

#include "message_queue/message_queue.hpp"
#include "peer/peer.hpp"

template <typename T> using shared_vector = std::shared_ptr<std::vector<T>>;
using shared_peer = std::shared_ptr<Peer>;

class Node
{
public:
	// start starts a node listening at address.
	// A negative value is returned for errors.
	int start(std::string address);

	// run starts the main processing routine.
	void run();

	Node()
	: id_counter(1),
	  mq(std::make_shared<Message_Queue>()),
	  peers_lock(std::make_shared<cpl::Mutex>()),
	  peers(std::make_shared<std::vector<shared_peer>>()),
	  close_notify_sem(std::make_shared<cpl::Semaphore>(0))
	{
	}

	// connect_to_peer connects to a peer with the given
	// address and creates a new Peer instance for the node.
	void
	connect_to_peer(cpl::net::SockAddr);

private:
	// process_message processes a single message in the inbound
	// message queue. This function blocks up to 33 milliseconds.
	void
	process_message();

	// cleanup_nodes runs in a separate thread to clean up
	// disconnected Peer instances.
	void
	cleanup_nodes();

	// handle_new_connections runs in a separate thread to
	// accept new connections.
	void
	handle_new_connections();

	// on_accept is the function called by handle_new_connections
	// whenever a new connection is accepted.
	void
	on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr);

	int id_counter;
	cpl::net::TCP_Socket sock;
	std::shared_ptr<Message_Queue> mq;

	// Vector of peers, protected by a mutex.
	std::shared_ptr<cpl::Mutex> peers_lock;
	shared_vector<shared_peer> peers;

	// close_notify_sem notifies the cleanup thread
	// whenever a Peer connection closes.
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
};
