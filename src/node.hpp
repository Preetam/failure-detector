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
	int start(std::string address);
	void run();

	Node()
	: id_counter(1),
	  mq(std::make_shared<Message_Queue>()),
	  peers_lock(std::make_shared<cpl::Mutex>()),
	  peers(std::make_shared<std::vector<shared_peer>>()),
	  close_notify_sem(std::make_shared<cpl::Semaphore>(0))
	{
	}

	void
	connect_to_peer(cpl::net::SockAddr);

private:
	void
	on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr);

	void
	cleanup_nodes();

	void
	handle_new_connections();

	int id_counter;
	cpl::net::TCP_Socket sock;
	std::shared_ptr<Message_Queue> mq;
	std::shared_ptr<cpl::Mutex> peers_lock;
	shared_vector<shared_peer> peers;
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
};
