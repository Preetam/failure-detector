#pragma once

#include <memory>

#include <cpl/net/tcp_socket.hpp>

#include "message_queue/message_queue.hpp"

class Node
{
public:
	int start(std::string address);
	void run();

	Node()
	{
		_mq = std::make_shared<Message_Queue>();
	}

private:
	void
	on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr);

	cpl::net::TCP_Socket _sock;
	std::shared_ptr<Message_Queue> _mq;
};
