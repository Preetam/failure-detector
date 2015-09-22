#pragma once

#include <memory>
#include <thread>
#include <iostream>
#include <future>

#include <cpl/tcp_connection.hpp>

#include "message.hpp"

class Peer
{
public:
	Peer(int id,
		 std::unique_ptr<cpl::net::TCP_Connection> conn_ptr,
		 std::packaged_task<void(int)> on_close_callback)
	: id(id),
	  conn_ptr(std::move(conn_ptr)),
	  on_close_callback(std::move(on_close_callback))
	{
	}

	void run() {
		thread = std::make_unique<std::thread>(&Peer::read_packets, this);
	}

	~Peer()
	{
		thread->join();
	}

	int id;

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn_ptr;
	std::unique_ptr<std::thread> thread;
	void read_packets();
	std::packaged_task<void(int)> on_close_callback;

}; // Peer
