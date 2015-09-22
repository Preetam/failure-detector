#pragma once

#include <thread>
#include <unordered_map>
#include <iostream>
#include <future>

#include <cpl/tcp_socket.hpp>

#include "peer.hpp"

class Node
{
public:
	int
	start(std::string address)
	{
		id_counter = 0;
		cpl::net::SockAddr addr;
		int status;
		status = addr.parse(address);
		if (status < 0) {
			return status;
		}

		status = _sock.bind(addr);
		if (status < 0) {
			return status;
		}
		status = _sock.listen();
		if (status < 0) {
			return status;
		}
		return 0;
	}

	void
	run() {
		while (true) {
			auto conn_ptr = std::make_unique<cpl::net::TCP_Connection>();
			if (_sock.accept(conn_ptr.get()) == 0) {
				on_accept(std::move(conn_ptr));
				id_counter++;
			} else {
			}
		}
	}
private:
	void
	on_accept(std::unique_ptr<cpl::net::TCP_Connection> conn_ptr)
	{
		auto peer = std::make_shared<Peer>(id_counter, std::move(conn_ptr),
			std::packaged_task<void(int)>([this](int id) {
				std::cerr << "connection " << id << " closed." << std::endl;
			}));
		peer->run();
		peers[id_counter] = std::move(peer);
		peers.clear();
	}


	cpl::net::TCP_Socket _sock;
	std::unordered_map<int,std::shared_ptr<Peer>> peers;
	int id_counter;
};
