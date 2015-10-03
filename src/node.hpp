#pragma once

#include <thread>
#include <vector>
#include <unordered_map>
#include <future>

#include <cpl/net/tcp_socket.hpp>

#include "peer.hpp"

class Node
{
public:
	int start(std::string address);
	void run();

private:
	void
	on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr);

	cpl::net::TCP_Socket _sock;
	std::shared_ptr<std::unordered_map<int,std::shared_ptr<Peer>>> peers;
	std::vector<std::unique_ptr<std::thread>> threads;
	int id_counter;
};
