#pragma once

#include <memory>
#include <thread>
#include <iostream>

#include <cpl/tcp_connection.hpp>

#include "message.hpp"

class Peer
{
public:
	Peer(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr)
	: conn_ptr(std::move(conn_ptr))
	{
	}

	void run() {
		thread = std::make_unique<std::thread>(&Peer::read_packets, this);
	}

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn_ptr;
	std::unique_ptr<std::thread> thread;

	void read_packets();

}; // Peer
