#pragma once

#include <memory>

#include <cpl/net/tcp_socket.hpp>

#include <cpl/rwmutex.hpp>

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
	  peers(std::make_shared<std::vector<unique_peer>>())
	{
	}

private:
	void
	on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr);

	cpl::net::TCP_Socket sock;
	std::shared_ptr<Message_Queue> mq;
	shared_vector<unique_peer> peers;
};
