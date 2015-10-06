#pragma once

#include <chrono>
#include <thread>
#include <memory>
#include <cerrno>

#include <cpl/net/tcp_socket.hpp>
#include <cpl/semaphore.hpp>
#include <cpl/mutex.hpp>

#include "message_queue/message_queue.hpp"
#include "peer/peer.hpp"

template <typename T> using shared_vector = std::shared_ptr<std::vector<T>>;
using unique_peer = std::unique_ptr<Peer>;

class Node
{
public:
	int start(std::string address);
	void run();

	Node()
	: mq(std::make_shared<Message_Queue>()),
	  peers_lock(std::make_shared<cpl::Mutex>()),
	  peers(std::make_shared<std::vector<unique_peer>>()),
	  close_notify_sem(std::make_shared<cpl::Semaphore>(0))
	{
	}

private:
	void
	on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr);

	void
	cleanup_nodes();

	cpl::net::TCP_Socket sock;
	std::shared_ptr<Message_Queue> mq;
	std::shared_ptr<cpl::Mutex> peers_lock;
	shared_vector<unique_peer> peers;
	std::shared_ptr<cpl::Semaphore> close_notify_sem;
};
