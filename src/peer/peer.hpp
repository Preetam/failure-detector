#pragma once

#include <memory>
#include <thread>
#include <iostream>

#include <cpl/net/tcp_connection.hpp>

#include "../message_queue/message_queue.hpp"

class Peer
{
public:
	Peer(std::unique_ptr<cpl::net::TCP_Connection> conn,
		std::shared_ptr<Message_Queue> mq)
	: conn(std::move(conn)), mq(mq)
	{
		std::cerr << "new peer connected" << std::endl;
	}

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn;
	std::unique_ptr<std::thread> thread;
	std::shared_ptr<Message_Queue> mq;
}; // Peer
