#pragma once

#include <memory>
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
		read_packets();
	}

	int id;

private:
	std::unique_ptr<cpl::net::TCP_Connection> conn_ptr;
	void read_packets();
	std::packaged_task<void(int)> on_close_callback;

}; // Peer
